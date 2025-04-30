# Micro-ROS Encoder Publisher for Mecanum Robot (ESP32)

This project is a micro-ROS node running on an ESP2 designed to read data from two motor encoders (e.g. rear Mecanum wheels) and broadcast this data to the ROS 2 system.

The ESP32 uses interrupt-based encoder reading for high accuracy and calculates RPM (rotations per minute) using the time between signal pulses.
---

## Features

- Accurate encoder reading using interrupts  
- Real-time RPM calculation  
- Publishes to the following ROS 2 topics:  
  - `/encoder2`: Raw encoder counts [motor3, motor4]  
  - `/velocity2`: Calculated RPMs [motor3, motor4]  
- Fully compatible with **ROS 2** (tested on Ubuntu 22.04 + ROS 2 Humble)

---

## Hardware Setup

- **Microcontroller**: ESP32  
- **Encoders**: the encoder I use has 4200 CPR quadrature
- **Encoder Pins** (In my setup):  
  - `ENCODER_A_3`, `ENCODER_B_3`: Motor 3 (rear left)  
  - `ENCODER_A_4`, `ENCODER_B_4`: Motor 4 (rear right)

> Update pin definitions in `encoder_pins.h` as needed:

```cpp
#define ENCODER_A_3 GPIO_NUM_XX
#define ENCODER_B_3 GPIO_NUM_XX
#define ENCODER_A_4 GPIO_NUM_XX
#define ENCODER_B_4 GPIO_NUM_XX
```

## Published ROS 2 Topics
Topic Name	Message Type	Description

  - /encoder2	std_msgs/msg/Float32MultiArray	Encoder tick counts [motor3, motor4]
  
  - /velocity2	std_msgs/msg/Float32MultiArray	RPM values [motor3, motor4]
  
## 🚀 Getting Started

### 1. Requirements

- [PlatformIO](https://platformio.org/)


# 2. Flash Code to ESP32
```bash 
git clone https://github.com/your-username/mecanum-encoder-micro-ros/.git
cd mecanum-encoder-micro-ros
```
- upload with vscode
  
# 3. Run micro-ROS Agent on Your ROS 2 Computer
```bash 
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyUSB0
```
Make sure to use the correct serial port (/dev/ttyUSB0, /dev/ttyUSB1, etc.).

# 4. View the Data in ROS 2
```bash 
ros2 topic echo /encoder2
ros2 topic echo /velocity2
```
##  Project Structure

```plaintext
.
├── src/
│   └── main.cpp              # Main firmware code
├── include/
│   └── encoder_pins.h        # Encoder pin definitions
├── platformio.ini            # PlatformIO config
└── README.md                 # This file

