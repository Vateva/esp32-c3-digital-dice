#ifndef MENU_H
#define MENU_H
#include <Arduino.h>
#include <esp_sleep.h>

//rtc variables struct
typedef struct {
    bool isValid;
    int coinCount;
    int d4Count;
    int d6Count;
    int d8Count;
    int d10Count;
    int d12Count;
    int d20Count;
    int brightness;
    int timeToClearDisplay;
    int stagger;
    int frameDelay;
    int rollHistory[6][8];
    int historyCount;
} RtcConfig;

extern RTC_DATA_ATTR RtcConfig rtcConfig;

//function prototypes
void openMenu();
//history variables
extern int rollHistory[6][8];
extern int historyCount;

//flash memory load and save functions
void saveConfiguration();
void loadConfiguration();

//getter functions for menu settings used by rollDice.cpp
int getNumberOfDice();
int getBrightness();
int getTimeToClearDisplay();
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