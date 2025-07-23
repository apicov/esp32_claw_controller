# ESP32 Claw Controller

## Overview
This project implements a controller for a claw machine using an ESP32 microcontroller. It features WiFi connectivity, MQTT communication, and PWM-based motor control for both the crane and the claw. The project is designed to be used with the Arduino framework and PlatformIO.

## Features
- Control a claw machine using MQTT messages
- WiFi connectivity for remote operation
- PWM control for smooth motor operation
- Limit switch support for safe movement
- Modular code structure
- Doxygen documentation support

## Hardware Requirements
- ESP32 development board (e.g., ESP32 DevKit v1)
- Claw machine with:
  - Two motors for crane movement (up/down)
  - One motor for claw open/close
  - Top and bottom limit switches
- Power supply suitable for motors and ESP32
- Wiring and connectors

## Software Requirements
- [PlatformIO](https://platformio.org/) (for building and uploading firmware)
- [Arduino framework](https://www.arduino.cc/)
- MQTT broker (e.g., Mosquitto)

## Getting Started

### 1. Clone the Repository
```sh
git clone https://github.com/yourusername/esp32_claw_controller.git
cd esp32_claw_controller
```

### 2. Configure WiFi and MQTT
Edit `include/private_data.h` to set your WiFi SSID, password, and MQTT broker details:
```cpp
#define SSID "your_wifi_ssid"
#define PASSWORD "your_wifi_password"
#define mqtt_broker "your_mqtt_broker_ip"
#define mqtt_port 1883
```

### 3. Build and Upload
Connect your ESP32 board and run:
```sh
platformio run --target upload
```

### 4. Monitor Serial Output
```sh
platformio device monitor
```

## Usage
- Send MQTT messages to the topic `claw/ctl` to control the claw:
  - `grab_seq <speed> <grip>`: Run grab sequence
  - `open`: Open the claw
  - `close <grip>`: Close the claw with specified grip strength
  - `up <speed>`: Move claw up
  - `down <speed>`: Move claw down
- The device publishes status updates to `claw/status`.

## Documentation
This project uses Doxygen for code documentation. To generate HTML docs:

1. Install Doxygen:
   ```sh
   sudo apt-get install doxygen
   ```
2. Run Doxygen in the project root:
   ```sh
   doxygen -g  # (first time only, to generate Doxyfile)
   doxygen
   ```
3. Open `html/index.html` in your browser.

## License
MIT License

## Author
- [Your Name](https://github.com/yourusername)
