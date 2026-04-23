#include "Scores.h"
#include "DisplayUtils.h"
#include <Preferences.h>    // ESP32 library to save data in Non-Volatile Storage

#ifndef BTN_UP
  #define BTN_UP    D9
  #define BTN_DOWN  D7
  #define BTN_LEFT  D10
  #define BTN_RIGHT D6
  #define BTN_CNT   D8 
#endif

#define EXIT_HOLD_TIME 2000

Preferences prefs;

// Hold scores
ScoreRecord highScores[3][10];
const char* diffNames[] = {"EASY", "NORMAL", "HARD"};

static int currentDifficultyView = 1;

// Handle "hold to exit"
static unsigned long exitHoldStartTime = 0;
static bool isHoldingExit = false;

static bool lastLeftBtn = false;
static bool lastRightBtn = false;

// Initialize scores
void initScores() {
  prefs.begin("scores", false);
  
  for (int d = 0; d < 3; d++) {
    for (int i = 0; i < 10; i++) {
      // d = difficulty, r = rank
      String key = "d" + String(d) + "r" + String(i);
      String val = prefs.getString(key.c_str(), "---,0");
      
      int commaIndex = val.indexOf(',');
      strcpy(highScores[d][i].name, val.substring(0, commaIndex).c_str());
      highScores[d][i].score = val.substring(commaIndex + 1).toInt();
    }
  }
  prefs.end();
}

// Main loop 
void drawScoresApp(bool &exitApp) {
  bool currLeft = digitalRead(BTN_LEFT) == HIGH;
  bool currRight = digitalRead(BTN_RIGHT) == HIGH;

  // Exit logic
  if (currLeft && currRight) {
    if (!isHoldingExit) {
      isHoldingExit = true;
      exitHoldStartTime = millis();
    } else if (millis() - exitHoldStartTime >= EXIT_HOLD_TIME) {
      exitApp = true;
      while(digitalRead(BTN_LEFT) == HIGH || digitalRead(BTN_RIGHT) == HIGH) { delay(10); }
      return; 
    }
  } else {
    isHoldingExit = false;
    
    // Navigation logic
    if (currRight && !lastRightBtn) {
      currentDifficultyView = (currentDifficultyView + 1) % 3;
    }
    if (currLeft && !lastLeftBtn) {
      currentDifficultyView = (currentDifficultyView == 0) ? 2 : currentDifficultyView - 1;
    }
  }
  
  lastLeftBtn = currLeft;
  lastRightBtn = currRight;

  // Rendering UI
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Draw the top 5 scores for the selected difficulty
  for (int i = 0; i < 5; i++) {
    int yPos = 12 + (i * 10); 

    // print rank and name
    display.setCursor(0, yPos);
    display.print(i + 1);
    display.print(".");
    display.print(highScores[currentDifficultyView][i].name);

    int scoreVal = highScores[currentDifficultyView][i].score;
    String scoreStr = String(scoreVal);
    int scoreWidth = scoreStr.length() * 6;
    display.setCursor(74 - scoreWidth, yPos); 
    display.print(scoreStr);
  }

  // Separator line
  display.drawFastVLine(78, 12, 52, SSD1306_WHITE);

  // Right side infos
  display.setCursor(89, 12);
  display.print("RANK:");

  // difficulty name
  String diffStr = diffNames[currentDifficultyView];
  int textX = 79 + (49 - (diffStr.length() * 6)) / 2; 
  display.setCursor(textX, 22);
  display.print(diffStr);

  display.setCursor(95, 32);
  display.print("< >");

  display.setCursor(89, 44);
  display.print("L + R");
  display.setCursor(92, 54);
  display.print("EXIT");

  // Exit progress bar rendering
  if (isHoldingExit) {
    unsigned long elapsed = millis() - exitHoldStartTime;
    int barWidth = (elapsed * SCREEN_WIDTH) / EXIT_HOLD_TIME; 
    
    display.fillRect(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12, SSD1306_BLACK);
    
    display.fillRect(0, SCREEN_HEIGHT - 3, barWidth, 3, SSD1306_WHITE);
    
    display.setCursor(35, SCREEN_HEIGHT - 11); 
    display.print("Exiting...");
  }

  display.display();
}

// Check if a given score is high enough
bool isHighScore(int score, int difficulty) {

  return score > highScores[difficulty][9].score;
}

// save the score
void saveNewScore(int score, int difficulty, const char* name) {

  int pos = 9;
  while (pos >= 0 && score > highScores[difficulty][pos].score) {
    pos--;
  }
  pos++;

  for (int i = 9; i > pos; i--) {
    highScores[difficulty][i] = highScores[difficulty][i-1];
  }

  // insert score and name
  strcpy(highScores[difficulty][pos].name, name);
  highScores[difficulty][pos].score = score;

  // Save the updated leaderboard to the NVS
  prefs.begin("scores", false);
  for (int i = 0; i < 10; i++) {
    String key = "d" + String(difficulty) + "r" + String(i);
    String val = String(highScores[difficulty][i].name) + "," + String(highScores[difficulty][i].score);
    prefs.putString(key.c_str(), val);
  }
  prefs.end();
}