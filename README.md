<div align="center">

# ☀️ Hybrid MPPT Solar Charge Controller Firmware
**Next-Generation ESP32 Cloud Telemetry Edition**

![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![Supabase](https://img.shields.io/badge/Supabase-3ECF8E?style=for-the-badge&logo=supabase&logoColor=white)

*A heavily optimized, cloud-connected fork of the original TechBuilder MPPT V2 firmware.*

</div>

---

## 📖 Overview
This repository contains the customized ESP32 firmware for the Hybrid MPPT Solar Charge Controller. Originally designed by Angelo S. Casimiro (TechBuilder), this specific fork has been massively upgraded to support **Global Cloud Telemetry** via Supabase REST APIs.

By completely stripping out the heavy, memory-intensive local embedded HTML server, we have freed up significant processing power on the ESP32. The hardware now acts as a pure, high-speed telemetry broadcaster—pushing data to the cloud where a beautiful React Dashboard handles the heavy lifting.

## ✨ Key Features
- ⚡ **True MPPT Algorithm**: High-speed Maximum Power Point Tracking using an IR2104 Synchronous Buck Converter.
- ☁️ **Global Cloud Sync**: Securely pushes 30+ telemetry data points (Voltage, Current, Power, Temperature, and Error States) to a Supabase PostgreSQL database every 5 seconds over HTTPS.
- 📡 **Remote Over-The-Air Settings**: Pulls configuration updates (MPPT/PSU Mode, Fan Thresholds, Voltage Limits) directly from the cloud.
- 🧠 **Dual-Core Architecture**: FreeRTOS is used to pin the heavy HTTPS networking stack to Core 0, ensuring the critical high-speed MPPT Buck Converter algorithms run entirely uninterrupted on Core 1.
- 🛡️ **Hardware Protections**: Built-in logic for Over-Voltage, Over-Current, Over-Temperature, and Fatally Low Voltage (FLV) automatic shutdowns.

## 🛠️ Hardware Requirements
- **Microcontroller**: ESP32 Dev Module
- **ADC**: ADS1015 (12-bit) or ADS1115 (16-bit) for precise voltage/current reading via I2C.
- **Display**: 16x2 I2C LCD Screen (Optional, can be disabled in code).
- **Thermal**: 100kΩ NTC Thermistor paired with a 10kΩ pull-down bias resistor connected to `GPIO 35`.
- **Power Stage**: Custom PCB with IR2104 half-bridge driver and MOSFETs.

## 🚀 Installation & Setup

### 1. Arduino IDE Setup
1. Download and install the [Arduino IDE](https://www.arduino.cc/en/software).
2. Install the ESP32 Board Manager.
3. Install the following libraries via the Library Manager:
   - `ArduinoJson` (v6.x - **Do not use v7 yet**)
   - `Adafruit_ADS1X15`
   - `LiquidCrystal_I2C`

### 2. Cloud Configuration
Open the `V2.ino` file and locate the Supabase Configuration section at the top. Insert your unique Project URL and Anon Key:

```cpp
// ========================================= SUPABASE CONFIG =======================================//
String supabaseUrl = "https://your-project-id.supabase.co";
String supabaseKey = "your-anon-key";
// =================================================================================================//
```

Locate the WiFi Configuration and insert your local network credentials:
```cpp
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";
```

### 3. Flashing to ESP32
1. Connect your ESP32 via USB.
2. Select **ESP32 Dev Module** from the Boards menu.
3. Set the baud rate to `115200`.
4. Click **Upload**. (Remember to hold the `BOOT` button on your ESP32 if it fails to connect during the "Connecting..." phase).

## ⚠️ Important Hardware Calibration
This firmware is pre-calibrated for a **100kΩ NTC Thermistor**. 
If your hardware uses a standard 10kΩ NTC Thermistor, you must change the calibration parameter in `V2.ino` (Line ~117):
```cpp
ntcResistance = 10000.00; // Change from 100000.00 to 10000.00
```
Failure to do so will result in an erroneous `140°C` reading at room temperature, which will trigger the `OTE` (Over-Temperature Error) safety shutdown!

## 📜 License & Credits
- **Original Hardware & Base Firmware**: Angelo S. Casimiro (TechBuilder)
- **Cloud Architecture & React Dashboard**: Open Source Custom Modification 
- **License**: MIT License
