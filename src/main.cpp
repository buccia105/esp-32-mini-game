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

// Button Definitions
#define BTN_CNT D8
#define BTN_UP D9
#define BTN_DOWN D7
#define BTN_LEFT D10
#define BTN_RIGHT D6

// System States
enum SystemState { STATE_MENU, STATE_SCORES, STATE_SNAKE, STATE_WEATHER };

SystemState currentState = STATE_MENU;
int menuSelection = 1;

// Variables for basic button edge detection
bool lastLeftState = LOW;
bool lastRightState = LOW;

// Variables for Deep Sleep & Center Button logic
unsigned long buttonPressStartTime = 0;
bool isHoldingCenter = false;
const unsigned long SHUTDOWN_TIME = 2000;

void goToSleep() {
  Serial.println("Preparazione allo spegnimento...");

  // 1. Mostra il feedback visivo di spegnimento
  drawShutdownScreen();

  // 2. Attesa obbligatoria del rilascio tasto (fondamentale per wake-up ext0)
  while (digitalRead(BTN_CNT) == HIGH) {
    delay(10);
  }

  // 3. Spegnimento software e hardware del display
  turnOffDisplay();
  delay(100); // Piccolo buffer per assicurare che il comando I2C sia inviato

  Serial.println("Display OFF. Deep Sleep avviato.");

  // 4. Configurazione risveglio e ingresso in Deep Sleep
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_CNT, 1);
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BTN_CNT, INPUT);
  pinMode(BTN_UP, INPUT);
  pinMode(BTN_DOWN, INPUT);
  pinMode(BTN_LEFT, INPUT);
  pinMode(BTN_RIGHT, INPUT);

  // Inizializzazione display
  initDisplay();

  // Controlla se il dispositivo si sta svegliando dal Deep Sleep
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("Risveglio rilevato.");

    // Mostra la schermata di benvenuto
    drawWakeupScreen();

    // La tiene visibile per 2 secondi
    delay(2000);
  }

  // Carica il menu principale
  drawMenu(menuSelection);
}

void loop() {
  bool leftPressed = digitalRead(BTN_LEFT) == HIGH;
  bool rightPressed = digitalRead(BTN_RIGHT) == HIGH;
  bool cntPressed = digitalRead(BTN_CNT) == HIGH;

  // --- LOGICA PULSANTE CENTRALE ---
  if (cntPressed) {
    if (!isHoldingCenter) {
      isHoldingCenter = true;
      buttonPressStartTime = millis();
    } else {
      if (millis() - buttonPressStartTime >= SHUTDOWN_TIME) {
        goToSleep();
      }
    }
  } else {
    if (isHoldingCenter) {
      unsigned long pressDuration = millis() - buttonPressStartTime;
      if (pressDuration < SHUTDOWN_TIME) {
        if (currentState == STATE_MENU) {
          // Switch to the selected app state
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

  // --- NAVIGAZIONE MENU ---
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

  // --- APP EXECUTION ---
  if (currentState == STATE_WEATHER) {
    bool exitApp = false;
    runWeatherApp(exitApp);

    if (exitApp) {
      currentState = STATE_MENU;
      drawMenu(menuSelection); // Redraw the menu when returning
    }
  }
  delay(20);
}