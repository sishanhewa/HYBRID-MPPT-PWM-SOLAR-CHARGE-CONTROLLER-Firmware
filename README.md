# Hybrid MPPT Solar Charge Controller (Cloud Firmware)

This repository contains the custom ESP32 firmware for the Hybrid MPPT Solar Charge Controller, originally designed by TechBuilder (Angelo S. Casimiro) and modified for **Cloud Telemetry via Supabase**.

## Key Features
- **True MPPT Algorithm**: Tracks the Maximum Power Point of the solar array.
- **Supabase Cloud Sync**: Pushes live telemetry (Voltage, Current, Power, Temperature, State) to a Supabase PostgreSQL database every 5 seconds.
- **Remote Settings Sync**: Pulls configuration updates (like MPPT Mode, Fan Temperature thresholds) directly from the cloud.
- **High Performance**: The heavy embedded HTML dashboard has been completely removed to free up ESP32 memory and processing power, migrating all UI responsibilities to a separate React web application.
- **Hardware Protections**: Over-voltage, over-current, and over-temperature safety shutdowns.

## Hardware Configuration
By default, the firmware expects the following hardware setup:
- ESP32 Microcontroller
- IR2104 Synchronous Buck Converter topology
- ADS1015/ADS1115 12-bit/16-bit ADC for precise voltage/current reading
- 100kΩ NTC Thermistor with a 10kΩ pull-down resistor for thermal monitoring

## Cloud Setup
To link the ESP32 to your cloud database, configure the following variables at the top of `V2.ino`:
```cpp
String supabaseUrl = "https://your-project-id.supabase.co";
String supabaseKey = "your-anon-key";
```

## Flashing Instructions
1. Open `V2.ino` in the Arduino IDE. (Do not open the numbered sub-files directly).
2. Install required libraries: `ArduinoJson` (v6.x), `Adafruit_ADS1X15`, `LiquidCrystal_I2C`.
3. Select "ESP32 Dev Module" as the board.
4. Click Upload.

## Architecture
The code utilizes the ESP32's dual-core architecture:
- **Core 0**: Handles the `WiFiClientSecure` HTTPS requests to Supabase to prevent blocking the charging algorithm.
- **Core 1**: Runs the high-speed MPPT Buck Converter PWM predictive algorithms and safety checks.
