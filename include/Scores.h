#ifndef SCORES_H
#define SCORES_H

#include <Arduino.h>

struct ScoreRecord {
  char name[4];
  int score;
};

void initScores();
void drawScoresApp(bool &exitApp);
bool isHighScore(int score, int difficulty);
void saveNewScore(int score, int difficulty, const char* name);

#endif