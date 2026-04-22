# ESP32 Mini Game Console 🎮⛅

A battery-powered, pocket-sized smart display and gaming console built on the Seeed Studio XIAO ESP32-S3. This project features a custom multi-app menu system, deep sleep power management, and live Wi-Fi data fetching.

## 🌟 Features
* **Custom UI:** A dynamic, inverted-color 3-app menu system (Scores, Snake, Weather).
* **Live Weather:** Connects to Wi-Fi to fetch real-time temperature and weather conditions via the OpenWeatherMap API.
* **Power Efficient:** Built-in "Hold to Shutdown" functionality that powers off the OLED panel and puts the ESP32 into Deep Sleep, waking up instantly via a hardware interrupt.
* **Object-Oriented Design:** Clean, modular C++ architecture managed via PlatformIO.

## 🛠️ Hardware Setup
* **Microcontroller:** Seeed Studio XIAO ESP32-S3
* **Display:** Adafruit 1.3" OLED (128x64px, I2C address `0x3D`)
* **Input:** 5x Tactile Push Buttons (with hardware pull-down resistors)

**Pin Configuration:**
* `D8` - Center (Select / Hold to Sleep / Wake)
* `D9` - Up
* `D7` - Down
* `D10` - Left (Menu Nav / Hold to Exit App)
* `D6` - Right (Menu Nav)

## 💻 Software & Installation
This project is built using **PlatformIO**.

### 1. Clone the repository
```bash
git clone https://github.com/buccia105/esp-32-mini-game.git
```

### 2. Configure Credentials
To protect sensitive data, Wi-Fi passwords and API keys are ignored by Git. You must create your own credentials file before compiling:

1. Navigate to the `include/` folder.
2. Create a new file named `credentials.h`.
3. Add the following code and insert your own details:
```cpp
#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#define SECRET_WIFI_SSID "Your_WiFi_Name"
#define SECRET_WIFI_PASS "Your_WiFi_Password"

#define SECRET_WEATHER_API_KEY "Your_OpenWeatherMap_API_Key"
#define SECRET_CITY "Bolzano"
#define SECRET_COUNTRY "IT"

#endif
```

### 3. Build and Upload
Open the project folder in VS Code with the PlatformIO extension. PlatformIO will automatically download the required dependencies (Adafruit GFX, Adafruit SSD1306, and ArduinoJson) based on the `platformio.ini` file. Click the **Upload** arrow to flash the board.

## 📜 License
This project is licensed under the [GNU GPLv3 License](LICENSE) - see the LICENSE file for details.
