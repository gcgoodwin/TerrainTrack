
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"
#include "camera_pins_reference.h"

const char* WIFI_SSID = "REPLACE_WITH_WIFI_SSID";
const char* WIFI_PASSWORD = "REPLACE_WITH_WIFI_PASSWORD";
const char* CONTROL_TOKEN = "REPLACE_WITH_LONG_RANDOM_SECRET";


constexpr int UNO_RX_GPIO = 41; // ESP32 RX <- Uno TX via 5V->3.3V level shifter
constexpr int UNO_TX_GPIO = 42; // ESP32 TX -> Uno RX
constexpr int LIDAR_RX_GPIO = 1; // ESP32 RX <- TFS20-L TX
constexpr int LIDAR_TX_GPIO = 2; // ESP32 TX -> TFS20-L RX (optional)

constexpr uint32_t COMMAND_WATCHDOG_MS = 800;
constexpr uint32_t LIDAR_STALE_MS = 400;
constexpr bool MOTION_ENABLED = false; 

HardwareSerial& unoUART = Serial1;
HardwareSerial& lidarUART = Serial2;
WebServer http(80);
uint8_t lidarFrame[9] = {};
int lidarIndex = 0;
uint16_t lidarCm = 0;
uint16_t lidarStrength = 0;
unsigned long lastLidar = 0;
unsigned long lastCommand = 0;
bool commandActive = false;
String unoTelemetry = "";
String unoLine = "";

void sendStop() {
  unoUART.print("S\n");
  commandActive = false;
}

bool authorized() {
  if (http.header("X-Api-Key") != CONTROL_TOKEN) {
    http.send(403, "text/plain", "Invalid control token");
    return false;
  }
  return true;
}

void handleControl() {
  if (!authorized()) return;
  if (!http.hasArg("speed") || !http.hasArg("steer") || !http.hasArg("pan") || !http.hasArg("tilt")) {
    http.send(400,"text/plain","Required: speed steer pan tilt"); return;
  }
  int speed = http.arg("speed").toInt();
  int steer = http.arg("steer").toInt();
  int pan = http.arg("pan").toInt();
  int tilt = http.arg("tilt").toInt();
  if (speed < -255 || speed > 255 || steer<0 || steer>180 || pan<0 || pan>180 || tilt<0 || tilt>180) {
    http.send(400,"text/plain","Value out of range"); return;
  }
  if (!MOTION_ENABLED) speed = 0; 
  if (lastLidar && millis()-lastLidar < LIDAR_STALE_MS && lidarCm < 80 && speed > 0) speed=0;

  unoUART.printf("M,%d,%d,%d,%d\n",speed,steer,pan,tilt);
  lastCommand=millis();
  commandActive=true;
  http.send(200,"application/json",String("{\"ok\":true,\"applied_speed\":")+speed+"}");
}

void handleCapture() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) { http.send(503,"text/plain","Camera capture failed"); return; }
  http.setContentLength(fb->len);
  http.send(200,"image/jpeg","");
  WiFiClient client=http.client();
  size_t sent=0;
  while (sent<fb->len && client.connected()) {
    size_t n=client.write(fb->buf+sent,min((size_t)1024,fb->len-sent));
    if (!n) break;
    sent+=n;
  }
  esp_camera_fb_return(fb);
}

void handleStatus() {
  const long age=lastLidar?(long)(millis()-lastLidar):-1;
  String json="{\"lidar_cm\":"+String(lidarCm)+",\"strength\":"+String(lidarStrength);
  json+=",\"lidar_age_ms\":"+String(age);
  json+=",\"motion_enabled\":"+String(MOTION_ENABLED?"true":"false");
  json+=",\"uno_telemetry\":\"";
  // Only allow safe printable serial characters in JSON string.
  for(unsigned i=0;i<unoTelemetry.length();i++) {
    char c=unoTelemetry[i];
    if(c=='"' || c=='\\') json+='\\';
    if(c>=32 && c<127) json+=c;
  }
  json+="\"}";
  http.send(200,"application/json",json);
}

void consumeLidarByte(uint8_t v) {
  if (lidarIndex==0 && v!=0x59) return;
  if (lidarIndex==1 && v!=0x59) {
    lidarIndex=(v==0x59)?1:0;
    return;
  }
  lidarFrame[lidarIndex++]=v;
  if(lidarIndex==9) {
    lidarIndex=0;
    uint8_t sum=0;
    for(int i=0;i<8;++i) sum+=lidarFrame[i];
    if(sum!=lidarFrame[8]) return;
    const uint16_t cm=(uint16_t)(lidarFrame[2]|(lidarFrame[3]<<8));
    if(cm<20 || cm>2000) return;
    lidarCm=cm;
    lidarStrength=(uint16_t)(lidarFrame[4]|(lidarFrame[5]<<8));
    lastLidar=millis();
  }
}

void consumeSensors() {
  while(lidarUART.available()) consumeLidarByte((uint8_t)lidarUART.read());
  while(unoUART.available()) {
    char c=(char)unoUART.read();
    if(c=='\n') {
      unoTelemetry=unoLine;
      unoLine="";
    } else if(c!='\r' && unoLine.length()<120) unoLine+=c;
  }
}

bool initCamera() {
  camera_config_t c={};
  c.ledc_channel=LEDC_CHANNEL_0;
  c.ledc_timer=LEDC_TIMER_0;
  c.pin_d0=Y2_GPIO_NUM; c.pin_d1=Y3_GPIO_NUM;
  c.pin_d2=Y4_GPIO_NUM; c.pin_d3=Y5_GPIO_NUM;
  c.pin_d4=Y6_GPIO_NUM; c.pin_d5=Y7_GPIO_NUM;
  c.pin_d6=Y8_GPIO_NUM; c.pin_d7=Y9_GPIO_NUM;
  c.pin_xclk=XCLK_GPIO_NUM; c.pin_pclk=PCLK_GPIO_NUM;
  c.pin_vsync=VSYNC_GPIO_NUM; c.pin_href=HREF_GPIO_NUM;
  c.pin_sccb_sda=SIOD_GPIO_NUM; c.pin_sccb_scl=SIOC_GPIO_NUM;
  c.pin_pwdn=PWDN_GPIO_NUM; c.pin_reset=RESET_GPIO_NUM;
  c.xclk_freq_hz=20000000;
  c.pixel_format=PIXFORMAT_JPEG;
  c.frame_size=FRAMESIZE_VGA; // 640x480 for modest person detection
  c.jpeg_quality=14;
  c.fb_count=psramFound()?2:1;
  c.fb_location=psramFound()?CAMERA_FB_IN_PSRAM:CAMERA_FB_IN_DRAM;
  c.grab_mode=CAMERA_GRAB_LATEST;
  return esp_camera_init(&c)==ESP_OK;
}

void setup() {
  Serial.begin(115200);
  unoUART.begin(115200,SERIAL_8N1,UNO_RX_GPIO,UNO_TX_GPIO);
  lidarUART.begin(115200,SERIAL_8N1,LIDAR_RX_GPIO,LIDAR_TX_GPIO);
  sendStop();
  if (!initCamera()) {
    Serial.println("Camera init FAILED. Verify exact board camera pinout.");
    while(true) { sendStop(); delay(1000); }
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  unsigned long start=millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-start<20000) delay(250);
  if(WiFi.status()!=WL_CONNECTED) {
    Serial.println("Wi-Fi connection failed: STOP mode");
    while(true) {sendStop(); delay(1000);}
  }
  const char* headers[]={"X-Api-Key"};
  http.collectHeaders(headers,1);
  http.on("/",HTTP_GET,[]{ http.send(200,"text/plain","TerrainTrack reference gateway: /capture /status /control /stop"); });
  http.on("/capture",HTTP_GET,handleCapture);
  http.on("/status",HTTP_GET,handleStatus);
  http.on("/control",HTTP_POST,handleControl);
  http.on("/stop",HTTP_POST,[]{if(!authorized())return;sendStop();http.send(200,"text/plain","stopped");});
  http.begin();
  Serial.print("TerrainTrack ESP IP: ");Serial.println(WiFi.localIP());
}

void loop() {
  consumeSensors();
  http.handleClient();
  if(commandActive && millis()-lastCommand>COMMAND_WATCHDOG_MS) sendStop();
  if(WiFi.status()!=WL_CONNECTED) sendStop();
  delay(1);
}
