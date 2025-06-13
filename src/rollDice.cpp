#include "coin_bitmaps_32.h"
#include "coin_bitmaps_64.h"
#include "d10_bitmaps_32.h"
#include "d10_bitmaps_64.h"
#include "d12_bitmaps_32.h"
#include "d12_bitmaps_64.h"
#include "d20_bitmaps_32.h"
#include "d20_bitmaps_64.h"
#include "d4_bitmaps_32.h"
#include "d4_bitmaps_64.h"
#include "d6_bitmaps_32.h"
#include "d6_bitmaps_64.h"
#include "d8_bitmaps_32.h"
#include "d8_bitmaps_64.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include "rollDice.h"
#include "menu.h"

//external references to objects from main.cpp
extern Adafruit_SH1106 display;

//structure to hold dice information
struct DiceInfo {
  int type;   //dice type: 0=coin, 4=d4, 6=d6, 8=d8, 10=d10, 12=d12, 20=d20
  int result; //rolled result
};

//function to get frame count per dice type
int getFramesPerDiceType(int diceType, int result = 0) {
  switch (diceType) {
  case 2:
    //coin animation varies by result
    if (result == 0) {
      return 42 + 7; //42 animation frames + 7 result frames = 49 total
    } else {
      return 28 + 7; //56 animation frames + 7 result frames = 63 total
    }
  default:
    return 18;
  }
}

//function to get appropriate bitmap array for dice type and size
const unsigned char *const *getDiceBitmapArray(int diceType, bool use64x64) {
  if (use64x64) {
    switch (diceType) {
    case 2:
      return coin_64_allArray;
    case 4:
      return d4_64_allArray;
    case 6:
      return d6_64_allArray;
    /*case 8: return d8_64_allArray;
    case 10: return d10_64_allArray;
    case 12: return d12_64_allArray;*/
    case 20: return d20_64_allArray;
    default:
      return d6_64_allArray; //fallback
    }
  } else {
    switch (diceType) {
    case 2:
      return coin_32_allArray;
    case 4:
      return d4_32_allArray;
    case 6:
      return d6_32_allArray;
    /*case 8: return d8_32_allArray;
    case 10: return d10_32_allArray;
    case 12: return d12_32_allArray;*/
    case 20: return d20_32_allArray;
    default:
      return d6_32_allArray; //fallback
    }
  }
}

void rollDice() {
  //gets current configuration settings
  int staggerTime = getStagger();
  int frameDelay = getframeDelay();

  //gets individual dice counts from menu settings
  int coinCount = getCoinCount();
  int d4Count = getD4Count();
  int d6Count = getD6Count();
  int d8Count = getD8Count();
  int d10Count = getD10Count();
  int d12Count = getD12Count();
  int d20Count = getD20Count();

  //calculates total number of dice
  int totalDice =
      coinCount + d4Count + d6Count + d8Count + d10Count + d12Count + d20Count;

  //early exit if no dice selected
  if (totalDice == 0) {
    return;
  }

  //builds ordered dice array from smallest to largest
  DiceInfo diceArray[8];
  int diceIndex = 0;

  //adds coins first (treated as 2-sided dice)
  for (int i = 0; i < coinCount && diceIndex < 8; i++) {
    diceArray[diceIndex].type = 2;              //coin = 2-sided
    diceArray[diceIndex].result = random(0, 2); //0-1 for heads/tails
    diceIndex++;
  }

  //adds d4 dice
  for (int i = 0; i < d4Count && diceIndex < 8; i++) {
    diceArray[diceIndex].type = 4;
    diceArray[diceIndex].result = random(1, 5); //1-4
    diceIndex++;
  }

  //adds d6 dice
  for (int i = 0; i < d6Count && diceIndex < 8; i++) {
    diceArray[diceIndex].type = 6;
    diceArray[diceIndex].result = random(1, 7); //1-6
    diceIndex++;
  }

  //adds d8 dice
  for (int i = 0; i < d8Count && diceIndex < 8; i++) {
    diceArray[diceIndex].type = 8;
    diceArray[diceIndex].result = random(1, 9); //1-8
    diceIndex++;
  }

  //adds d10 dice
  for (int i = 0; i < d10Count && diceIndex < 8; i++) {
    diceArray[diceIndex].type = 10;
    diceArray[diceIndex].result = random(1, 11); //1-10
    diceIndex++;
  }

  //adds d12 dice
  for (int i = 0; i < d12Count && diceIndex < 8; i++) {
    diceArray[diceIndex].type = 12;
    diceArray[diceIndex].result = random(1, 13); //1-12
    diceIndex++;
  }

  //adds d20 dice
  for (int i = 0; i < d20Count && diceIndex < 8; i++) {
    diceArray[diceIndex].type = 20;
    diceArray[diceIndex].result = random(1, 21); //1-20
    diceIndex++;
  }

  //store results in roll history (already ordered by dice size)
  //add to roll history
  if (historyCount < 6) {
    historyCount++;
  }

  //shift existing history down
  for (int i = historyCount - 1; i > 0; i--) {
    for (int j = 0; j < 8; j++) {
      rollHistory[i][j] = rollHistory[i - 1][j];
    }
  }

  //store new roll at index 0 (in dice size order)
  for (int i = 0; i < 8; i++) {
    if (i < totalDice) {
      rollHistory[0][i] = diceArray[i].result; //already in correct order
    } else {
      rollHistory[0][i] = -1; //clear unused slots
    }
  }

  //determines bitmap size and positioning based on total number of dice
  bool use64x64 = (totalDice <= 2);
  int bitmapSize = use64x64 ? 64 : 32;

  //calculates positions for each die
  int xPositions[8];
  int yPositions[8];
  calculateDicePositions(totalDice, use64x64, xPositions, yPositions);

  //find maximum frame count among all dice being rolled
  int maxFrames = 0;
  for (int i = 0; i < totalDice; i++) {
    int diceFrames =
        getFramesPerDiceType(diceArray[i].type, diceArray[i].result);
    if (diceFrames > maxFrames)
      maxFrames = diceFrames;
  }
  int totalFrames = ((totalDice - 1) * staggerTime) + maxFrames;

  //main animation loop with configurable stagger timing
  for (int globalFrame = 0; globalFrame < totalFrames; globalFrame++) {
    display.clearDisplay();

    for (int die = 0; die < totalDice; die++) {
      //gets appropriate bitmap array for this die's type
      const unsigned char *const *currentDieBitmapArray =
          getDiceBitmapArray(diceArray[die].type, use64x64);

      //calculate when this die should start animating
      int dieStartFrame = die * staggerTime;

      if (globalFrame >= dieStartFrame) {
        //this die has started animating
        int currentDieFrame = globalFrame - dieStartFrame;

        //calculate frames for this specific die type
        //calculate frames for this specific die type
        int framesForThisDie =
            getFramesPerDiceType(diceArray[die].type, diceArray[die].result);

        if (currentDieFrame < framesForThisDie) {
          //die is still animating
          int bitmapIndex;

          if (diceArray[die].type == 2) {
            //coin with result-based animation duration
            int animationFrames = (diceArray[die].result == 0) ? 42 : 28;

            if (currentDieFrame < animationFrames) {
              //coin is still animating - 28 frames per cycle
              bitmapIndex = (currentDieFrame) % 28;
            } else {
              //coin showing result frames
              int resultFrame = currentDieFrame - animationFrames;
              bitmapIndex = 28 + ((diceArray[die].result) * 7) + resultFrame;
            }
          } else {
            //other dice use standard animation
            if (currentDieFrame < 11) {
              bitmapIndex = currentDieFrame;
            } else {
              int resultFrame = currentDieFrame - 11;
              bitmapIndex =
                  11 + ((diceArray[die].result - 1) * 7) + resultFrame;
            }
          }

          display.drawBitmap(xPositions[die], yPositions[die],
                             currentDieBitmapArray[bitmapIndex], bitmapSize,
                             bitmapSize, WHITE);
        } else {
          //die has finished animating - show final result
          int finalBitmapIndex;
          if (diceArray[die].type == 2) {
            //coin final result frame
            finalBitmapIndex = 28 + ((diceArray[die].result) * 7) + 6;
          } else {
            //other dice final result frame
            finalBitmapIndex = 11 + ((diceArray[die].result -1) * 7) + 6;
          }
          display.drawBitmap(xPositions[die], yPositions[die],
                             currentDieBitmapArray[finalBitmapIndex],
                             bitmapSize, bitmapSize, WHITE);
        }
      }
    }

    display.display();
    delay(frameDelay);
  }

  //final display update showing all dice results
  display.clearDisplay();
  for (int die = 0; die < totalDice; die++) {
    const unsigned char *const *currentDieBitmapArray =
        getDiceBitmapArray(diceArray[die].type, use64x64);
    int finalBitmapIndex;
    if (diceArray[die].type == 2) {
      //coin final result frame
      finalBitmapIndex = 28 + ((diceArray[die].result) * 7) + 6;
    } else {
      //other dice final result frame
      finalBitmapIndex = 11 + ((diceArray[die].result - 1) * 7) + 6;
    }
    display.drawBitmap(xPositions[die], yPositions[die],
                       currentDieBitmapArray[finalBitmapIndex], bitmapSize,
                       bitmapSize, WHITE);
  }
  display.display();

  //save configuration including roll history to flash memory
  saveConfiguration();
}

void calculateDicePositions(int numDice, bool use64x64, int *xPos, int *yPos) {
  switch (numDice) {
  case 1:
    //single die centered
    xPos[0] = use64x64 ? 32 : 48; //center for 64x64 or 32x32
    yPos[0] = use64x64 ? 0 : 16;
    break;

  case 2:
    //two dice side by side
    if (use64x64) {
      xPos[0] = 0;
      yPos[0] = 0;
      xPos[1] = 64;
      yPos[1] = 0;
    } else {
      xPos[0] = 32;
      yPos[0] = 16;
      xPos[1] = 64;
      yPos[1] = 16;
    }
    break;

  case 3:
    //three dice in a row
    xPos[0] = 16;
    yPos[0] = 16;
    xPos[1] = 48;
    yPos[1] = 16;
    xPos[2] = 80;
    yPos[2] = 16;
    break;

  case 4:
    //2x2 grid
    xPos[0] = 16;
    yPos[0] = 0;
    xPos[1] = 80;
    yPos[1] = 0;
    xPos[2] = 16;
    yPos[2] = 32;
    xPos[3] = 80;
    yPos[3] = 32;
    break;

  case 5:
    //top row of 3, bottom row of 2 centered
    xPos[0] = 16;
    yPos[0] = 0;
    xPos[1] = 48;
    yPos[1] = 0;
    xPos[2] = 80;
    yPos[2] = 0;
    xPos[3] = 32;
    yPos[3] = 32;
    xPos[4] = 64;
    yPos[4] = 32;
    break;

  case 6:
    //3x2 grid
    xPos[0] = 16;
    yPos[0] = 0;
    xPos[1] = 48;
    yPos[1] = 0;
    xPos[2] = 80;
    yPos[2] = 0;
    xPos[3] = 16;
    yPos[3] = 32;
    xPos[4] = 48;
    yPos[4] = 32;
    xPos[5] = 80;
    yPos[5] = 32;
    break;

  case 7:
    //top row of 4, bottom row of 3 centered
    xPos[0] = 0;
    yPos[0] = 0;
    xPos[1] = 32;
    yPos[1] = 0;
    xPos[2] = 64;
    yPos[2] = 0;
    xPos[3] = 96;
    yPos[3] = 0;
    xPos[4] = 16;
    yPos[4] = 32;
    xPos[5] = 48;
    yPos[5] = 32;
    xPos[6] = 80;
    yPos[6] = 32;
    break;

  case 8:
    //4x2 grid (fills entire screen)
    xPos[0] = 0;
    yPos[0] = 0;
    xPos[1] = 32;
    yPos[1] = 0;
    xPos[2] = 64;
    yPos[2] = 0;
    xPos[3] = 96;
    yPos[3] = 0;
    xPos[4] = 0;
    yPos[4] = 32;
    xPos[5] = 32;
    yPos[5] = 32;
    xPos[6] = 64;
    yPos[6] = 32;
    xPos[7] = 96;
    yPos[7] = 32;
    break;
  }
}