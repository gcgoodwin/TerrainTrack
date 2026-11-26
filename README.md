# TerrainTrack

TerrainTrack is my project for an autonomous vehicle with obstacle aware tracking capabilities. This vehicle will aim to achieve this by mounting a camera and a LIDAR distance sensor on a modified remote control car that can drive autonomously. In this way, the vehicle will rely on this 3D scan of its surroundings to autonomously traverse through uneven terrain so that it can track and follow targets. I am building this because most simple robots with tracking capabilities can track a person/ moving object, but they don't necessarily understand whether the ground between them and that person is actually traversable. For example, someone walking down a sidewalk and stepping onto the road through a driveway ramp. A basic following robot might simply drive toward them, encounter a curb, and get stuck and get damaged. TerrainTrack aims to solve this problem by using cheap sensors to create a a 3D scan of its surroundings so that it can actively and autonomously plot courses that are terrain aware and traversable.

I’ve been working through the physical layout in Fusion 360. The 2 axis gimble mount needs to hold both the camera and the LIDAR range sensor. After finializing the sensors I am using, I designed the entire project in Fusion 360 CAD to simulate how individual parts would fit together. 
<img width="1546" height="1116" alt="IMG_4036" src="https://github.com/user-attachments/assets/b93e790b-6f79-49ec-9000-92ce2fa15e42" />



The gimble mount by itself:
<img width="968" height="1150" alt="IMG_1560" src="https://github.com/user-attachments/assets/daf0753b-f3dd-4154-903f-e2d2ea96345e" />

## How I want it to follow someone

The plan is to stream video over Wi-Fi from an ESP32 camera board to my MacBook.Once a person is selected in the video, my MacBook will determine the location of that person across the video. As the person moves left across the video, the pan servo can be used to pan the camera to follow the person. Commands to the motor and servos on the vehicle will be sent by an Arduino Uno R3.

I should also mention gimbal orientation. If I am standing next to the vehicle, I can have the gimbal rotate so that I am in the center of the frame and the steering wheels would still be facing straight ahead. The steering system will then aquire my location in the frame as well as the pan angle in order to determine a turn direction.

| Part | What I plan to use it for |
| --- | --- |
| Two micro servos | Pan the camera and LiDAR together and tilt them toward the ground for a scan. |
| DFRobot TFS20-L LiDAR | Measure distance along the direction the sensor is pointing. |
| BTS7960 motor driver | Let the Arduino control the rear brushed drive motor. |
| Wheel encoder and MPU-6050 | Estimate how far the chassis has traveled and how it moves between readings. The encoder still needs a mounting solution that fits the donor car. |
| Existing 7.4 V battery and voltage regulators | Power the drivetrain and supply the electronics at their required voltages. |

The parts list comes to **$124 before shipping and tax**. That assumes I reuse the chassis, battery, Arduino, and MacBook and have access to free 3D printing.

## Timeline

1. **Tracking with the car parked.** To do this, the car will be parked and I will Wwalk across the camera’s view and see whether the gimbal follows smoothly using the 2 servos. If I ever step out of the 180 deg FOV, I expect it to produce an error messege.
2. **Following on flat ground.** To trst this, I will walk slowly, turn, then stop. I want to see whether the chassis will drive to catch up with the camera’s heading and whether the car can hold its distance without repeatedly starting and stopping.
3. **Scanning a curb.** I will park the car near a curb and sweep the LiDAR across it. Each reading gives one distance, so the sensor will pan radar style and create a stationary 3D scan.

A limitation I’ll need to test is that the LiDAR moves with the camera. When the camera turns to follow someone beside the car, the sensor also turns away from the path in front of the wheels. This may not be a problem at longer ranges,but at shorter rages it could be bad so i might need to consider having 2 gimbles!

## Current progress

I have a CAD layout, a rough wiring diagram, a bill of materials, and reference firmware/code to work from. The first checkpoint I’m working on is simple enough to judge by watching it once: I will walk past the parked car, the camera follows me, and the wheels stay still the whole time. This is the planned wiring and power distribution of the entire system: 
https://github.com/gcgoodwin/TerrainTrack/blob/main/electronics/TERRAINTRACK_Wiring.drawio?utm_source=chatgpt.com

<img width="916" height="694" alt="Wiring" src="https://github.com/user-attachments/assets/1f44c556-e17a-4ec4-8172-a39b3e5823f4" />



## Bill of Materials (BOM)

| Component | Quantity | Unit Price (USD) | Line Total (USD) | Product Link | Status | Notes |
|---|---|---|---|---|---|---|
| ESP32 camera board with PSRAM | 1 | 17 | 17 | https://www.amazon.com/dp/B0GLWRBPJ4 | To purchase | Wi-Fi camera for MacBook vision processing |
| DFRobot TFS20-L ToF LiDAR | 1 | 44 | 44 | TFS20-L ToF LiDAR Sensor (20m, I2C/UART, 250Hz) - DFRobot | To purchase | Range measurements to create 3D scan |
| Micro servos (pan and tilt) | 2 | 5 | 10 | https://www.amazon.com/dp/B0CNL755KP | To purchase | Two units, $10 total |
| BTS7960 drive motor driver | 1 | 11 | 11 | https://www.amazon.com/dp/B00WSN98DC | To purchase | Rear brushed motor control |
| 5 V voltage regulator | 1 | 10 | 10 | https://www.amazon.com/dp/B0CQ1R55BV | To purchase | Regulated electronics/servo power |
| MPU-6050 IMU | 1 | 7 | 7 | https://www.amazon.com/dp/B01DK83ZYQ | To purchase | Inertial measurements |
| 3.3 V voltage regulator | 1 | 9 | 9 | https://www.amazon.com/dp/B0BVFYXPQ8 | To purchase | LiDAR power regulator to 3.3V |
| Wheel encoder kit | 1 | 16 | 16 | https://www.amazon.com/dp/B07JQQYQTF | To purchase | Wheel rotation / odometry |
| RC car chassis, drivetrain and suspension | 1 | 0 | 0 | | Already owned | |
| Arduino Uno R3 | 1 | 0 | 0 | | Already owned | Main motor/sensor controller |
| 7.4 V battery | 1 | 0 | 0 | | Already owned | Power source |
| MacBook | 1 | 0 | 0 | | Already owned | Vision processing |
| 3D-printed brackets and mounts | 1 | 0 | 0 | | Free | |
| Wires, connectors, screws | 1 | 0 | 0 | | Already owned | |
| PARTS SUBTOTAL | | | 124 | | | |
| Shipping | | | 0 | | Verify at checkout | 10% |
| Tax | | | 12.4 | | Verify at checkout | |
| Total | | | 136.4 | | | |


