
#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace Pins {
  const uint8_t ENC_LEFT = 2;
  const uint8_t ENC_RIGHT = 3;
  const uint8_t MOTOR_RPWM = 5;
  const uint8_t MOTOR_LPWM = 6;
  const uint8_t MOTOR_REN = 7;
  const uint8_t MOTOR_LEN = 8;
  const uint8_t STEER = 9;
  const uint8_t PAN = 10;
  const uint8_t TILT = 11;
}

const unsigned long FAILSAFE_MS = 700;
const unsigned long TELEMETRY_MS = 250;
const uint8_t MPU_ADDR = 0x68;

Servo steerServo, panServo, tiltServo;
volatile long leftTicks = 0;
volatile long rightTicks = 0;
char inputLine[64];
uint8_t inputIndex = 0;
unsigned long lastMotionFrame = 0;
unsigned long lastTelemetry = 0;
bool mpuReady = false;
bool timedOut = true;
int currentDrive = 0;

void onLeftPulse() { ++leftTicks; }
void onRightPulse() { ++rightTicks; }

void driveMotor(int value) {
  value = constrain(value, -255, 255);
  if (value == currentDrive) return;
  // Drive BOTH PWM pins low before switching direction.
  analogWrite(Pins::MOTOR_RPWM, 0);
  analogWrite(Pins::MOTOR_LPWM, 0);
  currentDrive = value;
  if (value > 0) analogWrite(Pins::MOTOR_RPWM, value);
  if (value < 0) analogWrite(Pins::MOTOR_LPWM, -value);
}

bool initMpu() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // PWR_MGMT_1
  Wire.write(0x00);  // Wake up
  return Wire.endTransmission() == 0;
}

bool readMpu(int16_t out[6]) {
  if (!mpuReady) return false;
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  // ACCEL_XOUT_H
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(MPU_ADDR, (uint8_t)14) != 14) return false;
  for (int i=0;i<3;++i) out[i] = (int16_t)((Wire.read()<<8)|Wire.read());
  Wire.read(); Wire.read();  // Temperature ignored
  for (int i=3;i<6;++i) out[i] = (int16_t)((Wire.read()<<8)|Wire.read());
  return true;
}

bool validRange(int n, int lo, int hi) { return n >= lo && n <= hi; }

void parseLine(char* text) {
  if (strcmp(text,"S") == 0) {
    driveMotor(0);
    timedOut = true;
    Serial.println(F("ACK,S"));
    return;
  }
  if (strcmp(text,"PING") == 0) {
    Serial.println(F("ACK,PONG"));
    return;
  }
  int drive, steer, pan, tilt;
  char extra;
  if (sscanf(text, "M,%d,%d,%d,%d%c", &drive, &steer, &pan, &tilt, &extra) != 4 ||
      !validRange(drive,-255,255) || !validRange(steer,0,180) ||
      !validRange(pan,0,180) || !validRange(tilt,0,180)) {
    driveMotor(0);
    Serial.println(F("ERR,BAD_FRAME"));
    return;
  }

  // Set the requested steering and gimbal angles before applying drive power.
  steerServo.write(steer);
  panServo.write(pan);
  tiltServo.write(tilt);
  lastMotionFrame = millis();
  timedOut = false;
  driveMotor(drive);
}

void readSerialLines() {
  while (Serial.available()) {
    const char ch = (char)Serial.read();
    if (ch == '\r') continue;
    if (ch == '\n') {
      inputLine[inputIndex] = 0;
      if (inputIndex) parseLine(inputLine);
      inputIndex = 0;
    } else if (inputIndex < sizeof(inputLine)-1) {
      inputLine[inputIndex++] = ch;
    } else {
      // Overflow invalidates the whole line; the next newline resets it.
      inputIndex = 0;
      driveMotor(0);
      Serial.println(F("ERR,FRAME_TOO_LONG"));
    }
  }
}

void sendTelemetry() {
  long l, r;
  noInterrupts(); l=leftTicks; r=rightTicks; interrupts();
  int16_t imu[6] = {};
  const bool ok = readMpu(imu);
  Serial.print(F("T,"));
  Serial.print(millis()); Serial.print(',');
  Serial.print(l); Serial.print(','); Serial.print(r);
  Serial.print(','); Serial.print(currentDrive);
  Serial.print(','); Serial.print(ok?1:0);
  if (ok) for (uint8_t i=0;i<6;++i) { Serial.print(','); Serial.print(imu[i]); }
  Serial.println();
}

void setup() {
  // Power wiring checked before connecting the motor or servo rails.
  pinMode(Pins::MOTOR_RPWM, OUTPUT);
  pinMode(Pins::MOTOR_LPWM, OUTPUT);
  pinMode(Pins::MOTOR_REN, OUTPUT);
  pinMode(Pins::MOTOR_LEN, OUTPUT);
  analogWrite(Pins::MOTOR_RPWM,0);
  analogWrite(Pins::MOTOR_LPWM,0);
  digitalWrite(Pins::MOTOR_REN,LOW);
  digitalWrite(Pins::MOTOR_LEN,LOW);
  Serial.begin(115200); // UNO hardware UART: D0 RX, D1 TX
  Wire.begin();
  Wire.setClock(100000);
  mpuReady=initMpu();
  pinMode(Pins::ENC_LEFT,INPUT_PULLUP);
  pinMode(Pins::ENC_RIGHT,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Pins::ENC_LEFT),onLeftPulse,FALLING);
  attachInterrupt(digitalPinToInterrupt(Pins::ENC_RIGHT),onRightPulse,FALLING);
  steerServo.attach(Pins::STEER);
  panServo.attach(Pins::PAN);
  tiltServo.attach(Pins::TILT);
  steerServo.write(90); panServo.write(90); tiltServo.write(90);
  // Only will enable the BTS7960 after outputs are defined and neutral.
  digitalWrite(Pins::MOTOR_REN,HIGH);
  digitalWrite(Pins::MOTOR_LEN,HIGH);
  lastMotionFrame=millis();
  Serial.println(F("READY,TERRAINTRACK_UNO"));
}

void loop() {
  readSerialLines();
  if (!timedOut && millis()-lastMotionFrame > FAILSAFE_MS) {
    driveMotor(0);
    timedOut=true;
    Serial.println(F("WARN,FAILSAFE_STOP"));
  }
  if (millis()-lastTelemetry >= TELEMETRY_MS) {
    lastTelemetry=millis();
    sendTelemetry();
  }
}
