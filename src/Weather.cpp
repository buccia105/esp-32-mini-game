#include "Weather.h"
#include "DisplayUtils.h"
#include "credential.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>


// state variables
bool isWifiConnected = false;
bool dataFetched = false;

float currentTemp = 0.0;
String weatherCondition = "Loading...";

// Variables for the Exit Logic
static unsigned long exitTimerStart = 0;
static bool isHoldingExit = false;
static bool weatherNeedsRedraw = true;
const unsigned long EXIT_HOLD_TIME = 2000;

void connectToWiFi() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(10, 20);
  display.print("Connecting Wi-Fi");
  display.display();

  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASS);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    display.print(".");
    display.display();
    attempts++;
  }

  display.clearDisplay();
  display.setCursor(10, 25);

  if (WiFi.status() == WL_CONNECTED) {
    isWifiConnected = true;
    display.print("Wi-Fi Connected!");
  } else {
    isWifiConnected = false;
    display.print("Connection Failed.");
  }

  display.display();
  delay(1500);
}

void fetchWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Construct the OpenWeatherMap URL (units=metric gives Celsius)
    String serverPath =
        "http://api.openweathermap.org/data/2.5/weather?q=" + 
        String(SECRET_CITY) + "," +
        String(SECRET_COUNTRY) + "&APPID=" + 
        String(SECRET_WEATHER_API_KEY) + "&units=metric";

    http.begin(serverPath.c_str());
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();

      // Parse the JSON data
      JsonDocument doc; // Dynamic JSON document for ArduinoJson v7
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        currentTemp = doc["main"]["temp"];

        // Get the main weather condition (e.g., "Clear", "Clouds", "Rain")
        const char *mainWeather = doc["weather"][0]["main"];
        weatherCondition = String(mainWeather);

        dataFetched = true;
      } else {
        weatherCondition = "JSON Error";
      }
    } else {
      weatherCondition = "HTTP Error: " + String(httpResponseCode);
    }

    http.end(); // Free resources
  }
}

void runWeatherApp(bool &exitApp) {
  // 1. Connection logic
  if (!isWifiConnected) {
    connectToWiFi();
  }

  // 2. Fetch Data (Only once after connecting)
  if (isWifiConnected && !dataFetched) {
    display.clearDisplay();
    display.setCursor(10, 25);
    display.print("fetching data...");
    display.display();

    fetchWeatherData();
    weatherNeedsRedraw = true; // Force redraw to show new data
  }

  // 3. Exit logic with BTN_LEFT (D10)
  bool leftPressed = digitalRead(D10) == HIGH;

  if (leftPressed) {
    if (!isHoldingExit) {
      isHoldingExit = true;
      exitTimerStart = millis();
    } else {
      unsigned long heldDuration = millis() - exitTimerStart;

      if (heldDuration > 200) {
        display.fillRect(10, 54, (heldDuration * 108) / EXIT_HOLD_TIME, 4,
                         SSD1306_WHITE);
        display.setCursor(10, 44);
        display.setTextColor(SSD1306_WHITE);
        display.setTextSize(1);
        display.print("exiting...");
        display.display();
      }

      if (heldDuration >= EXIT_HOLD_TIME) {
        exitApp = true;
        isHoldingExit = false;
        weatherNeedsRedraw = true;

        // Reset state so it fetches fresh data next time you open the app
        dataFetched = false;

        WiFi.disconnect();
        isWifiConnected = false;
        return;
      }
    }
  } else {
    if (isHoldingExit) {
      isHoldingExit = false;
      weatherNeedsRedraw = true;
    }
  }

  // 4. Draw the Interface
  // ... all'interno di runWeatherApp, nella parte di disegno dell'interfaccia ---
if (!isHoldingExit && weatherNeedsRedraw && dataFetched) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE); 
    
    // Nome Città
    display.setCursor(0, 6);
    display.setTextSize(1);
    display.print(String(SECRET_CITY));
    
    // Stato del meteo (testo)
    display.setCursor(0, 16);
    display.print(weatherCondition);
    
    // DISEGNO ICONA: Sotto il testo dello stato
    // Coord X: 0, Coord Y: 22 (subito sotto il testo)
    drawWeatherStatusIcon(0, 25, weatherCondition);
    
    // Temperatura (Spostata a destra per fare spazio all'icona)
    display.setCursor(45, 25);
    display.setTextSize(2);
    display.print(currentTemp, 1);
    display.print("C");
    
    display.setTextSize(1);
    display.setCursor(0, 54);
    display.print("Hold LEFT to Exit");
    
    display.display();
    weatherNeedsRedraw = false; 
}
}