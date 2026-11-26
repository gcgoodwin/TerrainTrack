---
title: "TerrainTrack"
github: "https://github.com/gcgoodwin/TerrainTrack"
description: "TerrainTrack is my project for an autonomous vehicle with obstacle aware tracking capabilities. This vehicle will aim to achieve this by mounting a camera and a LIDAR distance sensor on a modified remote control car that can drive autonomously. In this way, the vehicle will rely on this 3D scan of its surroundings to autonomously traverse through uneven terrain so that it can track and follow targets. I am building this because most simple robots with tracking capabilities can track a person/ moving object, but they don't necessarily understand whether the ground between them and that person is actually traversable. For example, someone walking down a sidewalk and stepping onto the road through a driveway ramp. A basic following robot might simply drive toward them, encounter a curb, and get stuck and get damaged. TerrainTrack aims to solve this problem by creating a #D scan of its surroundings so that it can plot courses that are terrain aware."
created_at: "2026-11-26"
total_time: "10h 30m"
---

# November 25, 2026: Initial RC Car Inspection and Steering Problem
<!-- fabricate:entry 3929 -->

Today, I got the idea of building a terrain aware autonomous vehicle by creating a 3D Scan. I found that most cars now on a budget that have tracking abilities are completely blind to its nearby terrain. Similar to a Tesla car, I aimed to design a cheap way for a car to track and follow moving objects while being aware of its terrain. or example, someone walking down a sidewalk and stepping onto the road through a driveway ramp. A basic following robot might simply drive toward them, encounter a curb, and get stuck and get damaged. Initially, I was thinking mounting everything a custom drone but I realized that with the lost cost budget I am going for, it would be impractical because of the issues with power, PID ... etc. Therefore I decided that a good platform for this would be a RC car that could be modified to drive autonomously. 
When I started inspecting the RC car I realized that the RC car had a steering issue in that the wheels were stuck in the left position and was completely unresponsive to the transmitter. 
At first I thought it might be a transmitter issue but I found that the car is able to drive forward so I took a few hours to completely dissect the car and find the root of the problem. Eventually I found that that a servo was responsible for this problem. I found that it was made up a motor and a potentiometer in which the motor was worn and broken. Debugging this took quite awhile but the solution was just a simple motor replacement and re soldering.


![](https://fabricate.hackclub-assets.com/3cd0b0b559e58908c17b5c550d1d0ff8401924ad2323f016b9ebf4615e33a1ea/image-jpeg/original_rc_control_pcb.jpg)
![](https://fabricate.hackclub-assets.com/cf17d1b0b3b98620d8f06c4d426c7739a4aec445692c3f7ad60c2bfa61e807d6/image-jpeg/steering_geartrain.jpg)
![](https://fabricate.hackclub-assets.com/8cc205c3d845c979ca161422fd3cbacfa8e9ca73848765c699332a9cd49a4677/image-jpeg/steering_feedback_sensor.jpg)
![](https://fabricate.hackclub-assets.com/227080b3c82bc013a72df0c7b7295379a5e29a856efcba9ee0f727ba8f2dd3a8/image-jpeg/steering_connector.jpg)

**Total time spent: 3h**

# November 25, 2026 (entry 2): CAD the RC Chassis
<!-- fabricate:entry 3930 -->

Once I found the problem with the RC Car, I started looking at how I could mount sensors on top of it. Before I could do this, I decided that it would be best and most appropriate for me to CAD the RC car on Fusion 360 before I designed the actual mount so that I could simulate the exact dimensions before I printed the mount. I started by taking dimensions of the car including the length, width and wheel diameters. I started with the overall shape of the chassis and the wheels. I then moved to the individual components such as the axles and suspensions that connect the wheels to the base. The steering mechanism was by far the most challenging part to CAD because of the pivot alignment issues I had with the servo. After doing the battery compartment, I decided to roughly design a mold for the PCB control board. Most importantly, I created a CAD of the car so that I could save the exact width and length of the chassis so that I can design a mount that can be custom fit to the chassis.



![](https://fabricate.hackclub-assets.com/a8a343b2d978b877b157907ef9be48c33d2672d6abbdc584ba6b224ed23a96c7/image-png/Screenshot%202026-11-25%20at%2010.06.18%E2%80%AFPM.png)
![](https://fabricate.hackclub-assets.com/ea6d648abcabbfb7ada08bf4d9e05dc92b7cc2f4a9f8e89b584495a1d470fc88/image-png/Screenshot%202026-11-25%20at%2010.05.52%E2%80%AFPM.png)
![](https://fabricate.hackclub-assets.com/8fc943fd3ebffcf53ca81e9774e5a0e0e87fbfd06a44ae20b1ef8fbdb0507038/image-png/Screenshot%202026-11-25%20at%2010.06.09%E2%80%AFPM.png)

**Total time spent: 3h 30m**

# November 25, 2026 (entry 3): Designing the sensor gimble
<!-- fabricate:entry 3931 -->

Today, I designed the actual mount for the components on top of the RC car. Using the engineering design process, I decided that I needed to figure out what exact capabilities I wanted the car to perform. To do this I finalized what exact sensors I was going to use to create a 3D scan. I found the cheapest way to do this is to use a camera combined with a LIDAR distance sensor. I learned how a LIDAR sensor shoots NIR light thousands of times a second to formulate a map millions of dots around itself to find out what the topography of the ground around itself is. I decided that an effective range for this sensor should be up to 20 meters. I found that the field of view of my LIDAR sensor is going to be less then 2 degrees meaning that at 2 meters the width of my stationary scan would be about 7 cm meaning that if I wanted to create a full 3d scan of the surroundings and constantly update it, I realized that I would need a radar style gimble for both the camera and Lidar sensors. To do this I decided that I should try to experiment with a 2 axis camera gimble using 2 MG90S servos. These servos only have a 180deg FOV because I figured that a car wouldn't really need to look behind itself. Once I finalized all of the sensors that I wanted to use and acquired the EXACT dimensions so I designed a custom structure that could gimble both sensors using the 2 servos that can be secured on the car. I wanted this mount to be removable so I designeded it so that there would be 2 permanent female screw connections glued to both edges of the car and the main structure could be screw onto it and therefore be removable. 
![](https://fabricate.hackclub-assets.com/93308d2c6210987d6f7a01a6c7b1ba97aaab77c2533e469ca9bc835405e267f7/image-png/Screenshot%202026-11-25%20at%2010.23.23%E2%80%AFAM.png)
![](https://fabricate.hackclub-assets.com/8ac473969403796f98efcef9403781577c06c27bc50b3963d697add23d522aac/image-png/Screenshot%202026-11-25%20at%2010.35.39%E2%80%AFAM.png)
![](https://fabricate.hackclub-assets.com/d9c94a9a0f4fb3e9f70a08ca3f99307eccf2137940b9923c86aab473e0be0848/image-png/Screenshot%202026-11-25%20at%2010.38.46%E2%80%AFAM.png)

**Total time spent: 4h**
