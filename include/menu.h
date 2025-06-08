/*
 * menu.h - Header file for menu system
 */

#ifndef MENU_H
#define MENU_H

// Function prototypes
void openMenu();

// Getter functions for menu settings
int getNumberOfDice();
int getSelectedDiceType();
int getBrightness();
int getClearDisplayTime();
int getAnimationFrames();
int getAnimationFPS();

#endif // MENU_H