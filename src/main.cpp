// main include
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>
#include <esp_sleep.h>

// helper files
#include "DisplayUtils.h"
#include "Scores.h"
#include "Snake.h"
#include "Weather.h"

// pinout definitions
#define BTN_CNT D8
#define BTN_UP D9
#define BTN_DOWN D7
#define BTN_LEFT D10
#define BTN_RIGHT D6

// system states for app selection
enum SystemState { STATE_MENU, STATE_SCORES, STATE_SNAKE, STATE_WEATHER };

SystemState currentState = STATE_MENU;
int menuSelection = 1;

// input edge detection
bool lastLeftState = LOW;
bool lastRightState = LOW;

// center button logic and sleep variables
unsigned long buttonPressStartTime = 0;
bool isHoldingCenter = false;
const unsigned long SHUTDOWN_TIME = 2000; // time to hold the cnt button for deep sleep initiation

// sleep function
void goToSleep() {
  Serial.println("Preparazione allo spegnimento...");

  // feedback to user
  drawShutdownScreen();

  // wait for relese
  while (digitalRead(BTN_CNT) == HIGH) {
    delay(10);
  }

  // turn off the display
  turnOffDisplay();
  delay(100); // buffer to ensure i2c command acknowledge

  Serial.println("Display OFF. Deep Sleep avviato.");

  // configuration for wake-up
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_CNT, 1);
  esp_deep_sleep_start();
}

// SETUP
void setup() {
  // serial configuration
  Serial.begin(115200);
  delay(1000);

  // pinout configuration
  pinMode(BTN_CNT, INPUT);
  pinMode(BTN_UP, INPUT);
  pinMode(BTN_DOWN, INPUT);
  pinMode(BTN_LEFT, INPUT);
  pinMode(BTN_RIGHT, INPUT);

  // display initialization
  initDisplay();

  // check for wake-up
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("Risveglio rilevato.");

    // user greeting
    drawWakeupScreen();
    delay(2000);
  }

  // main menu vizualization
  drawMenu(menuSelection);
}

// MAIN LOOP
void loop() {
  bool leftPressed = digitalRead(BTN_LEFT) == HIGH;
  bool rightPressed = digitalRead(BTN_RIGHT) == HIGH;
  bool cntPressed = digitalRead(BTN_CNT) == HIGH;

  // CENTER BUTTON LOGIC
  // check for center press
  if (cntPressed) {
    // verify long press
    if (!isHoldingCenter) {
      isHoldingCenter = true;
      buttonPressStartTime = millis();
    } 
    else {
      if (millis() - buttonPressStartTime >= SHUTDOWN_TIME) {
        goToSleep(); // initiate sleep
      }
    }
  } 
  
  else {
    if (isHoldingCenter) {
      unsigned long pressDuration = millis() - buttonPressStartTime;
      // check for short press
      if (pressDuration < SHUTDOWN_TIME) {
        if (currentState == STATE_MENU) {
          // switch system state
          if (menuSelection == 0)
            currentState = STATE_SCORES;
          else if (menuSelection == 1)
            currentState = STATE_SNAKE;
          else if (menuSelection == 2)
            currentState = STATE_WEATHER;
        }
      }
      isHoldingCenter = false;
    }
  }

  // menu navigation
  if (currentState == STATE_MENU && !isHoldingCenter) {
    bool menuChanged = false;

    if (rightPressed && !lastRightState) {
      menuSelection = (menuSelection + 1) % 3;
      menuChanged = true;
    }

    if (leftPressed && !lastLeftState) {
      menuSelection = (menuSelection == 0) ? 2 : menuSelection - 1;
      menuChanged = true;
    }

    if (menuChanged) {
      drawMenu(menuSelection);
    }
  }

  lastLeftState = leftPressed;
  lastRightState = rightPressed;

  // APP EXECUTION
  if (currentState == STATE_WEATHER) {
    bool exitApp = false;
    runWeatherApp(exitApp, BTN_LEFT);

    if (exitApp) {
      currentState = STATE_MENU;
      drawMenu(menuSelection); // Redraw the menu when returning
    }
  }
  delay(20);
}