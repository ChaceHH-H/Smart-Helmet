# Smart Safety Helmet
## Overview
The smart safety helmet project aims to provide real-time health and environmental monitoring for miners. By integrating multiple sensors (including heart rate sensors, gas sensors, pressure sensors, and accelerometers), the helmet can monitor the miners' vital signs and the safety of the surrounding environment in real time, and issue an alarm when dangerous situations are detected. The project uses Arduino and Firebase for data processing and storage to ensure the timeliness and reliability of information.
### Question
How to keep miners safe through smart helmets with integrated environmental sensors?
### Hypothesis
Smart safety helmets: Improving miner safety by integrating vital signs and environmental monitoring sensors
## Installation
### Hardware
- Arduino R3 and Arduino R4 WiFi
- MAX30102 heart rate sensor
- MICS-5524 gas sensor
- GY-61 ADXL335 accelerometer
- DF9-40 High Precise Resistance pressure sensor
- RGB LED
- Buzzer
- 9V battery
### Library
- Arduino IDE
- SparkFun MAX301x Particle Sensor
- ArduinoJson 
- NTPClient 
- DFRobot_MICS
### Schematic
![Schematic](https://github.com/ChaceHH-H/Image/blob/main/helmet_bb.png)
![Schematic](https://github.com/ChaceHH-H/Image/blob/main/%E5%A4%B4%E7%9B%941.jpg)
### How to get started
- Install all the hardware according to the schematic.
- Open Arduino IDE and install the required libraries
- Upload HeartRate-r3.ino and helmet-r4.ino to the corresponding Arduino board.
- Configure Firebase, create a Firebase project and enable Firestore database.
- In the helmet-r4.ino file, update the Firebase API Key and project ID.
- Open the web dashboard to view the data.
## User Guide
- Start the device: Make sure all sensors are connected correctly and turn on the power switch of the battery compartment.
- Wi-Fi connection: Connect to wifi when the LED light is white.
- Gas sensor preheating: When the LED is yellow, wait for 3 minutes to preheat, and turn green to indicate that the preheating is complete.
- Wearing the helmet: Wear the helmet correctly and then try each function.
- View data: Open the web interface to view real-time data.
