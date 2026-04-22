#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

extern Adafruit_SSD1306 display;

void initDisplay();
void drawMenu(int selectedIndex);
void drawShutdownScreen();
void turnOffDisplay();
void drawWakeupScreen();
void drawWeatherStatusIcon(int x, int y, String condition);

#endif