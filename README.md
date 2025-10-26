# DIY Earthquake Alert System

A DIY **Earthquake Alert System** built with the ESP8266 NodeMCU ESP12E and an MPU6050 6-axis accelerometer + gyroscope module.  
The system continuously monitors vibrations, filters noise, and activates both an audible and visual alarm (buzzer + LED) when shaking is detected.

---

## Video Demonstration

Watch the live demo here:  
[ Video Demonstration](https://vt.tiktok.com/ZSUXM96RY/)  

---

## Disclaimer 

This project is a **DIY Earthquake Alert System** created for learning, experimentation, and hobby use. It is not a certified earthquake detection or early warning system and should not be relied upon for safety-critical applications. Professional, industrial, and government systems are far more advanced and undergo rigorous testing.

Use this project as a hands-on learning tool to explore sensors, IoT, and electronics, not as a substitute for real-world disaster prevention systems.

The author is not responsible for any damage, injury, or loss caused by using this project. Always handle electronics safely.

---

## Hardware Requirements

- ESP8266 development board NodeMCU ESP12E  
- MPU6050 accelerometer + gyroscope sensor  
- Passive buzzer  
- Jumper wires
- breadboard 

---

## Circuit Diagrams

- Basic Setup:

![Circuit Diagram](./img/Schematic_simple.png)

## MCU to Sensor Wiring

| NodeMCU Pin | Function / Signal | Destination           |
|-------------|------------------|-----------------------|
| 3V3         | Power VCC        | MPU6050 VCC           |
| GND         | Ground           | MPU6050 GND           |
| D1 (GPIO5)  | I²C SCL          | MPU6050 SCL           |
| D2 (GPIO4)  | I²C SDA          | MPU6050 SDA           |
| D5 (GPIO14) | Buzzer +         | Passive Buzzer (+)    |
| GND         | Buzzer −         | Passive Buzzer (−)    |
| D4 (GPIO2)  | Onboard LED      | LED indicator         |

- Solar Powered Setup:

![Circuit Diagram](./img/solar.png)


---

## Setup & Instructions

### 1. Install Arduino IDE
- Download and install the latest [Arduino IDE](https://www.arduino.cc/en/software).

### 2. Add ESP8266 Board Package
1. Open Arduino IDE  
2. Go to **File → Preferences**  
3. In **Additional Board Manager URLs**, add:
> http://arduino.esp8266.com/stable/package_esp8266com_index.json  
4. Go to **Tools → Board → Board Manager**, search for **ESP8266**, and install.

### 3. Install Required Libraries
In Arduino IDE, go to **Sketch → Include Library → Manage Libraries...** and install:
- **Wire** (usually pre-installed)  
- **MPU6050** by Electronic Cats / Jeff Rowberg (I²Cdevlib)  

### 4. Select Your Board & Port
- Go to **Tools → Board**, select your ESP8266 NodeMCU ESP12E 1.0.  
- Select the correct **COM Port** under **Tools → Port**.  

### 5. Upload the Code
1. Open the provided `.ino` code file in Arduino IDE.  
2. Click the **Upload** button (right arrow icon).  
3. Wait until the code compiles and uploads successfully.  

### 6. Test the System
- Once uploaded, open the **Serial Monitor** at **115200 baud**.  
- The system will calibrate automatically, then start monitoring vibrations.  
- Shake the sensor slightly to trigger the buzzer + LED.  

---

## Author

- Created with passion ❤ by **Roy Cuadra** 
- Updated Date: 10-18-2025

---

**Thank you for checking out this project!** 
You are welcome to **fork**, **improve**, or **use** it for learning purposes.
