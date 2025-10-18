Earthquake Alert System (ESP8266 + MPU6050)
==========================================

This is a DIY Earthquake Alert System using the ESP8266 (e.g. NodeMCU / Wemos D1 Mini) and a 3‑axis IMU sensor (MPU6050). 
The system monitors vibrations, filters noise, and triggers an audible + visual alarm (buzzer + LED) when a threshold is exceeded.

Features
--------
- Baseline calibration to ignore static offsets
- Moving average filtering to smooth out noise
- Consecutive‐trigger logic (to reduce false alarms)
- Pulsing buzzer + LED during alert
- Cooldown and failsafe auto‑reset mechanisms
- Error handling for sensor disconnection

Hardware Requirements
---------------------
- ESP8266 board (NodeMCU, Wemos D1 Mini, etc.)
- MPU6050 accelerometer / gyroscope module
- Passive buzzer
- (Uses onboard LED)
- Wires, breadboard, power supply

Pinout & Wiring
---------------
| ESP8266 Pin | Component / Signal  | Destination          |
|-------------|----------------------|----------------------|
| 3V3         | VCC                 | MPU6050 VCC          |
| GND         | GND                 | MPU6050 GND          |
| D1 (GPIO5)  | I²C SCL             | MPU6050 SCL          |
| D2 (GPIO4)  | I²C SDA             | MPU6050 SDA          |
| D5 (GPIO14) | Buzzer +            | Buzzer positive       |
| GND         | Buzzer −            | Buzzer negative       |
| D4 (GPIO2)  | Onboard LED         | (active LOW)         |

Notes:
- The onboard LED on many ESP8266 dev boards is tied to GPIO2 (D4) and is active LOW.
- The buzzer should be a passive buzzer.
- Ensure stable 3.3 V supply for the MPU6050.
