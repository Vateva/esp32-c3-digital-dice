/*
 * rollDice.h - header file for dice rolling functionality
 */

#ifndef ROLLDICE_H
#define ROLLDICE_H

//main function prototypes
void rollDice();

//helper function prototypes for positioning
void calculateDicePositions(int numDice, bool use64x64, int* xPos, int* yPos);

//bitmap array selection function
const unsigned char* const* getDiceBitmapArray(int diceType, bool use64x64);

#endif //ROLLDICE_H