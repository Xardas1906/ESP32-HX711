### README for Torque Wrench Web Interface Program

#### Overview

This program implements a web-based interface for a torque wrench measurement system using an ESP32 microcontroller. It leverages various technologies including HTML, CSS, JavaScript, WebSocket communication, and server-side C++ code to provide real-time torque measurements, calibration capabilities, and historical data visualization.

#### Features

- **Real-Time Measurements:** Displays torque measurements in real-time on a web-based gauge.
- **Calibration:** Allows users to calibrate the torque wrench using known weights and adjust the calibration factor.
- **Data Visualization:** Graphs historical torque data dynamically.
- **WebSocket Communication:** Uses WebSocket for real-time communication between the ESP32 and the web interface.
- **Error Handling and Notifications:** Provides feedback on WebSocket connectivity and errors directly through the web interface.

#### Components

1. **main.cpp**: Contains the ESP32 firmware. Manages WiFi and WebSocket server, handles sensor readings from the HX711 load cell, and stores calibration data using EEPROM.
2. **index.html**: The main HTML document that structures the web interface.
3. **style.css**: Defines the visual style and layout of the web interface.
4. **script.js**: Contains all client-side JavaScript logic for updating the web interface, handling user inputs, and managing WebSocket messages.,
5. **gauge.min.js** and **chart.js**: JavaScript libraries used for rendering the gauge and chart.
6. **jquery-3.7.1.min.js**: jQuery library for DOM manipulations.

#### Configuration

- **Network Setup:** Configured to create a WiFi access point with predefined SSID and password.
- **WebServer and WebSocket Server:** Runs an HTTP server on port 80 and a WebSocket server on port 81.

#### Installation

1. **Prerequisites:**
   - ESP32 board.
   - HX711 load cell amplifier.
   - Basic knowledge of electronics and programming.
   - PlatformIO IDE for ESP32 firmware upload.

2. **Setting up the environment:**
   - Install PlatformIO IDE.
   - Open PlatformIO and create a new project.
   - Select the ESP32 board from the list of devices.
   - Add the required libraries (`HX711`, `WiFi`, `WebServer`, `FS`, `SPIFFS`, `WebSocketsServer`, `EEPROM`) via the PlatformIO Library Manager.

3. **Uploading the firmware:**
   - Navigate to the project directory.
   - Place the `main.cpp` file into the `src` folder of your PlatformIO project.
   - Ensure all HTML, CSS, and JS files are in the `data` folder for SPIFFS.
   - Compile the project using PlatformIO's build button.
   - Upload the code and file system image using the upload buttons.

4. **Loading the web interface:**
   - After uploading, the ESP32 will host a WiFi network.
   - Connect to this network using a web browser.
   - Navigate to `http://esp32.local` to access the web interface.

#### Usage

- **Measure Torque:** View real-time torque measurements on the gauge.
- **Set Calibration:** Enter a known weight and press "Calibrate" to update the calibration factor.
- **Historical Data:** View past torque measurements on a line chart.
- **Adjust Settings:** Use the interface buttons to reset calibration, save calibration factors, or tare the scale.

#### Troubleshooting

- **Connectivity Issues:** Ensure your device is connected to the ESP32 WiFi network. Check the IP and MDNS configurations.
- **WebSocket Errors:** If WebSocket errors occur, check the console for error messages and ensure the server code is running correctly on the ESP32.
- **Sensor Problems:** Verify connections between ESP32 and HX711, and ensure correct pin assignments in `main.cpp`.

This README provides a basic guide to setting up and using the torque wrench measurement system with PlatformIO.
