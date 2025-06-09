/*
 * menu.h - header file for menu system
 */

#ifndef MENU_H
#define MENU_H
#include <Arduino.h>

//function prototypes
void openMenu();
extern void setDisplayBrightness(uint8_t brightness);
//external history variables from rollDice.cpp
extern int rollHistory[6][8];
extern int historyCount;
void saveConfiguration();
void loadConfiguration();

//getter functions for menu settings used by rollDice.cpp
int getNumberOfDice();
int getBrightness();
int getClearDisplayTime();
int getStagger();
int getframeDelay(); //note: matches existing function name in menu.cpp

//new getter functions for individual dice counts
int getCoinCount();
int getD4Count();
int getD6Count();
int getD8Count();
int getD10Count();
int getD12Count();
int getD20Count();

#endif //MENU_H