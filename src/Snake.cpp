#include "Snake.h"
#include "DisplayUtils.h"
#include "Scores.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Buttons definition
#ifndef BTN_UP
  #define BTN_UP    D9
  #define BTN_DOWN  D7
  #define BTN_LEFT  D10
  #define BTN_RIGHT D6
  #define BTN_CNT   D8 
#endif

// Display and game grid configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BLOCK_SIZE 4 
#define MAX_SNAKE_LENGTH 100 
#define Y_OFFSET 12     
#define BOTTOM_MARGIN 10 


#define EXIT_HOLD_TIME 2000   // Exit time 2s 

#define GRID_W (SCREEN_WIDTH / BLOCK_SIZE)
#define GRID_H ((SCREEN_HEIGHT - Y_OFFSET - BOTTOM_MARGIN) / BLOCK_SIZE)

// Snake state variables
static int snakeX[MAX_SNAKE_LENGTH];
static int snakeY[MAX_SNAKE_LENGTH];
static int snakeLength = 3;
static int current_dir = 1; // 0: Up, 1: Right, 2: Down, 3: Left
static int next_dir = 1;    

// Input state
static bool lastUp = false, lastDown = false, lastLeft = false, lastRight = false;
static int foodX, foodY;
static int score = 0;

// Difficulty and speed settings
static int difficultyLevel = 1;
static int currentSpeed = 150;
const int speeds[3] = {250, 150, 80};

// Variables for exit hold logic
static unsigned long exitHoldStartTime = 0;
static bool isHoldingExit = false;

// Score name variables
static bool enteringName = false;
static int nameCharIdx = 0;
static char initials[4] = "AAA";
static bool checkedHighScore = false;


static int gameState = 0; 
static unsigned long lastMoveTime = 0; 
static bool isInitialized = false;

// Randomly generate coordinates for the food
void spawnFood() {
  foodX = random(1, GRID_W - 1);
  foodY = random(1, GRID_H - 1);
}
 // Reset game
void resetGame() {
  snakeLength = 3;
  current_dir = 1;
  next_dir = 1;

  // Center the snake initially
  snakeX[0] = GRID_W / 2;     snakeY[0] = GRID_H / 2;
  snakeX[1] = snakeX[0] - 1;  snakeY[1] = snakeY[0];
  snakeX[2] = snakeX[0] - 2;  snakeY[2] = snakeY[0];
  
  score = 0;
  spawnFood();
  gameState = 1; // play
  checkedHighScore = false;
  
  // Reset button states
  lastUp = false; lastDown = false; lastLeft = false; lastRight = false;
  isHoldingExit = false;
  
  // Show "Ready"
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(45, 28 + Y_OFFSET);
  display.print("READY!");
  display.display();
  delay(800); 
  lastMoveTime = millis();
}

// Main loop
void runSnakeApp(bool &exitApp) {

  if (!isInitialized) {
    gameState = 0; // Start at the main menu
    isInitialized = true;
    randomSeed(millis()); 
    isHoldingExit = false;
    checkedHighScore = false;
  }

  // State: 0 -- Main menu
  if (gameState == 0) {
    bool currLeft = digitalRead(BTN_LEFT) == HIGH;
    bool currRight = digitalRead(BTN_RIGHT) == HIGH;

    // Difficulty selection
    if (currLeft && !lastLeft) {
      difficultyLevel--;
      if (difficultyLevel < 0) difficultyLevel = 2;
    }
    if (currRight && !lastRight) {
      difficultyLevel++;
      if (difficultyLevel > 2) difficultyLevel = 0;
    }
    
    lastLeft = currLeft;
    lastRight = currRight;

    // Update speed based on the difficulty selected
    currentSpeed = speeds[difficultyLevel];
    
    // Menu UI
    display.clearDisplay();
    display.setTextSize(2); 
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(30, 12); 
    display.print("SNAKE");
    

    display.setTextSize(1); 
    display.setCursor(30, 28);
    if (difficultyLevel == 0)      display.print("< EASY >");
    else if (difficultyLevel == 1) display.print("< NORMAL >");
    else if (difficultyLevel == 2) display.print("< HARD >");

    display.setCursor(30, 48); 
    display.print("CNT to Play");
    display.display(); 

    // Start game
    if(digitalRead(BTN_CNT) == HIGH) {
      while(digitalRead(BTN_CNT) == HIGH) { delay(10); } 
      resetGame();
    }
  } 

  // State 1: Gameplay
  else if (gameState == 1) {
    bool currUp = digitalRead(BTN_UP) == HIGH;
    bool currDown = digitalRead(BTN_DOWN) == HIGH;
    bool currLeft = digitalRead(BTN_LEFT) == HIGH;
    bool currRight = digitalRead(BTN_RIGHT) == HIGH;

    // Update next direction, preventing 180 degree turns
    if (currUp && !lastUp && current_dir != 2)       next_dir = 2; 
    if (currRight && !lastRight && current_dir != 3) next_dir = 1; 
    if (currDown && !lastDown && current_dir != 0)   next_dir = 0; 
    if (currLeft && !lastLeft && current_dir != 1)   next_dir = 3; 

    lastUp = currUp; lastDown = currDown; lastLeft = currLeft; lastRight = currRight;


    if (millis() - lastMoveTime >= currentSpeed) {
      lastMoveTime = millis();
      current_dir = next_dir; 

      for (int i = snakeLength - 1; i > 0; i--) {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
      }
      
      // Move the head based on current direction
      if (current_dir == 0) snakeY[0]--;
      if (current_dir == 1) snakeX[0]++;
      if (current_dir == 2) snakeY[0]++;
      if (current_dir == 3) snakeX[0]--;

      // check for wall collisions
      if (snakeX[0] < 1 || snakeX[0] >= GRID_W - 1 || 
          snakeY[0] < 1 || snakeY[0] >= GRID_H - 1) {
        gameState = 2;  // trigger game over
      }

      // check for self collision
      for (int i = 1; i < snakeLength; i++) {
        if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) gameState = 2; 
      }

      // Check if food is eaten
      if (snakeX[0] == foodX && snakeY[0] == foodY) {
        if (snakeLength < MAX_SNAKE_LENGTH) {
          // Grow the snake
          snakeX[snakeLength] = snakeX[snakeLength - 1];
          snakeY[snakeLength] = snakeY[snakeLength - 1];
          snakeLength++;
        }
        score += 10;
        spawnFood();    // generate food
      }


      if (gameState == 1) {
        display.clearDisplay();
        
        // Draw play area boundary
        display.drawRect(0, Y_OFFSET, SCREEN_WIDTH, SCREEN_HEIGHT - Y_OFFSET - BOTTOM_MARGIN, SSD1306_WHITE);
        // Draw food
        display.fillRect(foodX * BLOCK_SIZE, (foodY * BLOCK_SIZE) + Y_OFFSET, BLOCK_SIZE, BLOCK_SIZE, SSD1306_WHITE);
        // Draw snake
        for (int i = 0; i < snakeLength; i++) {
          display.fillRect(snakeX[i] * BLOCK_SIZE, (snakeY[i] * BLOCK_SIZE) + Y_OFFSET, BLOCK_SIZE, BLOCK_SIZE, SSD1306_WHITE);
        }
        
        // Draw score UI
        display.setTextSize(1);
        display.setCursor(SCREEN_WIDTH - 70, SCREEN_HEIGHT - 8); 
        display.print("SCORE: "); display.print(score);
        display.display();
      }
    }
  }

  // State 2: Game over
  else if (gameState == 2) {
    
    // Check if player achieved a high score
    if (!checkedHighScore) {
      checkedHighScore = true;
      if (isHighScore(score, difficultyLevel)) {
        enteringName = true;
        nameCharIdx = 0;
        strcpy(initials, "AAA");
      } else {
        enteringName = false;
      }
    }

    // Name entry screen
    if (enteringName) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(15, 10);
      display.print("NEW HIGH SCORE!");
      

      for(int i = 0; i < 3; i++) {
        display.setTextSize(2);
        display.setCursor(40 + (i * 18), 28);
        display.print(initials[i]);

        if(nameCharIdx == i) {
          display.drawFastHLine(40 + (i * 18), 45, 10, SSD1306_WHITE);
        }
      }
      
      display.setTextSize(1);
      display.setCursor(10, 55);
      display.print("UP/DN:Char CNT:Save");
      display.display();

      // Hande character selection inputs
      if (digitalRead(BTN_UP) == HIGH) {
        initials[nameCharIdx]++;
        if (initials[nameCharIdx] > 'Z') initials[nameCharIdx] = 'A';
        delay(150);
      }
      if (digitalRead(BTN_DOWN) == HIGH) {
        initials[nameCharIdx]--;
        if (initials[nameCharIdx] < 'A') initials[nameCharIdx] = 'Z';
        delay(150);
      }
      if (digitalRead(BTN_RIGHT) == HIGH || digitalRead(BTN_LEFT) == HIGH) {
        nameCharIdx = (nameCharIdx + 1) % 3;
        delay(200);
      }
      // Save score and name
      if (digitalRead(BTN_CNT) == HIGH) {
        saveNewScore(score, difficultyLevel, initials);
        enteringName = false;
        while(digitalRead(BTN_CNT) == HIGH) { delay(10); } 
      }
    } 

    else {
      bool currLeft = digitalRead(BTN_LEFT) == HIGH;
      bool currRight = digitalRead(BTN_RIGHT) == HIGH;

      // Left + Rigth to exit app logic
      if (currLeft && currRight) {
        if (!isHoldingExit) {
          isHoldingExit = true;
          exitHoldStartTime = millis();
        } else if (millis() - exitHoldStartTime >= EXIT_HOLD_TIME) {
          exitApp = true;
          isInitialized = false;

          while(digitalRead(BTN_LEFT) == HIGH || digitalRead(BTN_RIGHT) == HIGH) { delay(10); }
          return; 
        }
      } else {
        isHoldingExit = false;
      }

      // Game over UI
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(12, 15);
      display.print("GAME OVER");
      
      display.setTextSize(1);
      display.setCursor(35, 38);
      display.print("Score: "); display.print(score);
      

      display.setCursor(10, 52);
      display.print("CNT=Play L+R=Exit"); 

      // Draw progress bar while exit
      if (isHoldingExit) {
        unsigned long elapsed = millis() - exitHoldStartTime;
        int barWidth = (elapsed * SCREEN_WIDTH) / EXIT_HOLD_TIME; 
        

        display.fillRect(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12, SSD1306_BLACK);
        display.fillRect(0, SCREEN_HEIGHT - 3, barWidth, 3, SSD1306_WHITE);
        display.setCursor(35, SCREEN_HEIGHT - 11); 
        display.print("Exiting...");
      }

      display.display(); 
      
      // Return to game menu
      if(digitalRead(BTN_CNT) == HIGH && !isHoldingExit) {
        while(digitalRead(BTN_CNT) == HIGH) { delay(10); } 

        gameState = 0; 
        delay(200); 
      }
    }
  }
}