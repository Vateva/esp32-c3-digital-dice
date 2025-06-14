#include "menu.h"
#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Fonts/TomThumb.h>
#include <Preferences.h>
#include <Wire.h>

//external references to main.cpp objects
extern Adafruit_SH1106 display;
extern unsigned long lastButtonPress;
extern bool buttonPressed;
int rollHistory[6][8];
int historyCount;

//preferences object for flash storage
Preferences preferences;

int currentMenu = MENU_MAIN;

int currentSelection = 0;

//main menu option labels
const char *mainMenuItems[MAIN_MENU_ITEMS] = {"Dice", "Roll history",
                                              "Configuration", "Exit"};
//dice menu option labels - updated with coin and individual counts
const char *diceMenuItems[DICE_MENU_ITEMS] = {"Coin", "D4",  "D6",  "D20",
                                              "D8",   "D10", "D12", "Back"};
//config menu option labels
const char *configMenuItems[CONFIG_MENU_ITEMS] = {
    "Brightness", "Time to clear", "Stagger time", "Frame delay(ms)", "Back"};

//individual dice counters - replaces single numberOfDice variable
int coinCount = 0;
int d4Count = 0;
int d6Count = 1; //default to 1 d6
int d8Count = 0;
int d10Count = 0;
int d12Count = 0;
int d20Count = 0;

//default configuration values
int brightness = BRIGHTNESS_DEFAULT;
int timeToClearDisplay = TIME_TO_CLEAR_DISPLAY_DEFAULT; //milliseconds
int stagger = STAGGER_DEFAULT;
int frameDelay = FRAME_DELAY_DEFAULT;

//timing variables for button handling
unsigned long buttonPressStart = 0;
bool longPressDetected = false;
bool menuExitRequested = false;

//function prototypes
void drawMenu();
void handleButtonPress();
bool isButtonPressed();
void executeMenuAction();
int getTotalDiceCount();
int getMaxAllowedCount(int currentTypeCount);
int *getDiceCountPointer(int menuIndex);
void saveConfiguration();
void loadConfiguration();

void openMenu() {
  //loads configuration from flash memory
  loadConfiguration();

  //resets menu state when opening
  currentMenu = MENU_MAIN;
  currentSelection = 0;
  buttonPressed = true;
  longPressDetected = true;
  lastButtonPress = millis();
  buttonPressStart = 0;
  menuExitRequested = false;

  //draws initial menu
  drawMenu();

  //main menu loop - stays here until exit is selected
  while (!menuExitRequested) {
    handleButtonPress();
    delay(10); //small delay to prevent excessive cpu usage
  }

  //saves configuration to flash memory before exiting
  saveConfiguration();
}

void drawMenu() {
  //clears display for fresh menu render
  display.clearDisplay();

  //sets text properties for menu display
  display.setTextColor(WHITE);
  display.setTextSize(1);

  switch (currentMenu) {
  case MENU_MAIN:
    //draws menu title at top of screen
    display.setCursor(37, 5);
    display.println("Main menu");

    //draws each menu item with selection indicator
    for (int i = 0; i < MAIN_MENU_ITEMS; i++) {
      display.setCursor(0,
                        15 + (i * 13)); //positions each item 12 pixels apart

      //draws selection indicator for current item
      if (i == currentSelection) {
        display.print(">");
      } else {
        display.print(" "); //spaces for alignment when not selected
      }

      //draws the menu item text
      display.print(mainMenuItems[i]);
    }
    break;
  case MENU_HISTORY:
    //draws roll history menu title
    display.setCursor(15, 5);
    display.println("  Roll History");

    //checks if there are any rolls to display
    if (historyCount == 0) {
      //shows message when no history exists
      display.setCursor(20, 25);
      display.println("No rolls yet");
    } else {
      //switches to small font for history entries
      display.setFont(&TomThumb);

      //determines how many entries to show (max 6)
      int entriesToShow = (historyCount > 6) ? 6 : historyCount;

      //draws each history entry
      for (int i = 0; i < entriesToShow; i++) {
        display.setCursor(0,
                          19 + (i * 7)); //positions each entry 7 pixels apart

        //formats entry label based on age
        if (i == 0) {
          display.print("Last: ");
        } else {
          display.print(" n-");
          display.print(i + 1);
          display.print("  : ");
        }

        //gets the roll to display (newest first)
        int rollIndex = i;

        //counts non-minus one dice in this roll
        int diceCount = 0;
        for (int j = 0; j < 8; j++) {
          if (rollHistory[rollIndex][j] != -1) {
            diceCount++;
          }
        }

        //displays dice values separated by commas
        for (int j = 0; j < diceCount; j++) {
          display.print(rollHistory[rollIndex][j]);
          if (j < diceCount - 1) {
            display.print("-");
          }
        }
      }

      //switches back to default font for back button
      display.setFont();
    }

    //draws back button with selection indicator
    display.setCursor(0, 56);
    if (currentSelection == 0) {
      display.print(">");
    } else {
      display.print(" ");
    }
    display.print("Back");
    break;

  case MENU_DICE:
    //draws dice menu title
    display.setCursor(37, 5);
    display.println("Dice Menu");

    //draws dice menu items in two columns with counts
    for (int i = 0; i < DICE_MENU_ITEMS; i++) {
      int col = i / 4; //0 for left column, 1 for right column
      int row = i % 4; //row within column (0-3)

      display.setCursor(col * 64,
                        17 + (row * 13)); //64 pixels apart for columns

      //draws selection indicator for current item
      if (i == currentSelection) {
        display.print(">");
      } else {
        display.print(" ");
      }

      //draws the menu item text
      display.print(diceMenuItems[i]);

      //show current count for each dice type (not for "Back")
      if (i < DICE_MENU_ITEMS - 1) {
        display.print(":");
        switch (i) {
        case 0: //coin
          display.print(coinCount);
          break;
        case 1: //d4
          display.print(d4Count);
          break;
        case 2: //d6
          display.print(d6Count);
          break;
        case 3: //d20
          display.print(d20Count);
          break;
        case 4: //d8
          display.print(d8Count);
          break;
        case 5: //d10
          display.print(d10Count);
          break;
        case 6: //d12
          display.print(d12Count);
          break;
        }
      }
    }
    break;

  case MENU_CONFIG:
    //draws configuration menu title
    display.setCursor(25, 5);
    display.println("Configuration");

    //draws config menu items with selection indicator
    for (int i = 0; i < CONFIG_MENU_ITEMS; i++) {
      display.setCursor(0, 16 + (i * 10)); //positions each item

      //draws selection indicator for current item
      if (i == currentSelection) {
        display.print(">");
      } else {
        display.print(" ");
      }

      //draws the menu item text
      display.print(configMenuItems[i]);

      //show current values for config items (not for "Back")
      if (i < CONFIG_MENU_ITEMS - 1) {
        display.print(":");
        switch (i) {
        case 0: //brightness
          display.print(map(brightness, 0, 255, 0, 100));
          display.print("%");
          break;
        case 1: //clear display time
          display.print(timeToClearDisplay / 1000);
          display.print("s");
          break;
        case 2: //animation frames
          display.print(stagger);
          break;
        case 3: //animation fps
          display.print(frameDelay);
          break;
        }
      }
    }
    break;
  }

  //updates physical display with menu content
  display.display();
}

void handleButtonPress() {
  bool currentButtonState = isButtonPressed();
  unsigned long currentTime = millis();

  //detects button press start
  if (currentButtonState && !buttonPressed) {
    //checks debounce timing to avoid false triggers
    if (currentTime - lastButtonPress > DEBOUNCE_DELAY_MS) {
      buttonPressed = true;
      buttonPressStart = currentTime;
      longPressDetected = false;
    }
  }

  //detects long press while button is held
  if (buttonPressed && currentButtonState) {
    if (currentTime - buttonPressStart >= LONG_PRESS_MENU_SELECTION &&
        !longPressDetected) {
      longPressDetected = true;
      executeMenuAction();
    }
  }

  //detects button release for navigation
  if (!currentButtonState && buttonPressed) {
    buttonPressed = false;
    lastButtonPress = currentTime;

    //only navigates if it wasn't a long press selection
    if (!longPressDetected) {
      //moves to next menu item based on current menu
      switch (currentMenu) {
      case MENU_MAIN:
        currentSelection = (currentSelection + 1) % MAIN_MENU_ITEMS;
        break;
      //no case for MENU_HISTORY since no navigation
      case MENU_DICE:
        currentSelection = (currentSelection + 1) % DICE_MENU_ITEMS;
        break;
      case MENU_CONFIG:
        currentSelection = (currentSelection + 1) % CONFIG_MENU_ITEMS;
        break;
      }

      //redraws menu with new selection
      drawMenu();
    }
  }
}

//function to set display brightness/contrast
void setDisplayBrightness(uint8_t brightness) {
  //sh1106 uses contrast command for brightness
  //send command sequence to set contrast
  Wire.beginTransmission(0x3C); //display i2c address
  Wire.write(0x00);             //command mode
  Wire.write(0x81);             //set contrast command
  Wire.write(brightness);       //contrast value 0-255
  Wire.endTransmission();
}

//calculates total number of dice across all types
int getTotalDiceCount() {
  return coinCount + d4Count + d6Count + d8Count + d10Count + d12Count +
         d20Count;
}

//calculates maximum allowed count for cycling a specific dice type
int getMaxAllowedCount(int currentTypeCount) {
  int totalOthers = getTotalDiceCount() - currentTypeCount;
  return 8 - totalOthers;
}

//returns pointer to the count variable for a given menu index
int *getDiceCountPointer(int menuIndex) {
  switch (menuIndex) {
  case 0:
    return &coinCount;
  case 1:
    return &d4Count;
  case 2:
    return &d6Count;
  case 3:
    return &d20Count;
  case 4:
    return &d8Count;
  case 5:
    return &d10Count;
  case 6:
    return &d12Count;
  default:
    return nullptr;
  }
}

void executeMenuAction() {
  switch (currentMenu) {
  case MENU_MAIN:
    switch (currentSelection) {
    case 0: //dice menu
      currentMenu = MENU_DICE;
      currentSelection = 0;
      drawMenu();
      break;
    case 1: //configuration menu
      currentMenu = MENU_HISTORY;
      currentSelection = 0;
      drawMenu();
      break;
    case 2: //configuration menu
      currentMenu = MENU_CONFIG;
      currentSelection = 0;
      drawMenu();
      break;
    case 3: //exit
      menuExitRequested = true;
      break;
    }
    break;

  case MENU_DICE:
    //handles dice count cycling for all dice types
    if (currentSelection < DICE_MENU_ITEMS - 1) { //not "Back"
      int *countPtr = getDiceCountPointer(currentSelection);
      if (countPtr != nullptr) {
        int currentCount = *countPtr;
        int maxAllowed = getMaxAllowedCount(currentCount);

        //cycle count from 0 to maxAllowed
        *countPtr = (currentCount + 1) % (maxAllowed + 1);

        drawMenu();
      }
    } else { //back option
      currentMenu = MENU_MAIN;
      currentSelection = 0;
      drawMenu();
    }
    break;
  case MENU_HISTORY:
    //back button (only option)
    currentMenu = MENU_MAIN;
    currentSelection = 1; //return to "Roll history" option
    drawMenu();
    break;
  case MENU_CONFIG:
    switch (currentSelection) {
    case 0: //brightness
      //cycle through brightness levels
      if (brightness == 192) {
        brightness = 255; //force 100% after 75%
      } else if (brightness == 255) {
        brightness = 64; //wrap back to 25% after 100%
      } else {
        brightness = brightness + 64; //normal increment
      }
      //apply brightness to display
      setDisplayBrightness(brightness);
      drawMenu();
      break;
    case 1: //time to clear
      //cycle through times (s)
      timeToClearDisplay =
          (timeToClearDisplay >= 5000) ? 1000 : timeToClearDisplay + 500;
      drawMenu();
      break;
    case 2: //stagger time
      //cycle through stagger time
      stagger = (stagger == 5) ? 0 : stagger + 1;
      drawMenu();
      break;
    case 3: //frame delay(ms)
      frameDelay = (frameDelay == 75) ? 0 : frameDelay + 5;
      drawMenu();
      break;
    case 4: //back
      currentMenu = MENU_MAIN;
      currentSelection = 1; //return to configuration option
      drawMenu();
      break;
    }
    break;
  }
}

bool isButtonPressed() {
  //reads button state - returns true when button is pressed
  //button is active low due to pullup resistor configuration
  return digitalRead(BUTTON_PIN) == LOW;
}

//functions to access menu settings from main.cpp and rolldice.cpp
int getNumberOfDice() { return getTotalDiceCount(); }

int getBrightness() { return brightness; }

int getTimeToClearDisplay() { return timeToClearDisplay; }

int getStagger() { return stagger; }

int getframeDelay() { return frameDelay; }

//new getter functions for individual dice counts
int getCoinCount() { return coinCount; }
int getD4Count() { return d4Count; }
int getD6Count() { return d6Count; }
int getD8Count() { return d8Count; }
int getD10Count() { return d10Count; }
int getD12Count() { return d12Count; }
int getD20Count() { return d20Count; }

//saves all configuration to flash memory
void saveConfiguration() {
  preferences.begin("diceConfig", false); //false = read/write mode

  //save dice counts
  preferences.putInt("coinCount", coinCount);
  preferences.putInt("d4Count", d4Count);
  preferences.putInt("d6Count", d6Count);
  preferences.putInt("d8Count", d8Count);
  preferences.putInt("d10Count", d10Count);
  preferences.putInt("d12Count", d12Count);
  preferences.putInt("d20Count", d20Count);

  //save configuration settings
  preferences.putInt("brightness", brightness);
  preferences.putInt("clearTime", timeToClearDisplay);
  preferences.putInt("stagger", stagger);
  preferences.putInt("frameDelay", frameDelay);

  //save roll history
  preferences.putBytes("rollHistory", rollHistory, sizeof(rollHistory));
  preferences.putInt("historyCount", historyCount);

  preferences.end();
}

//loads all configuration from flash memory
void loadConfiguration() {
  preferences.begin("diceConfig", true); //true = read-only mode

  //load dice counts with defaults if not found
  coinCount = preferences.getInt("coinCount", 0);
  d4Count = preferences.getInt("d4Count", 0);
  d6Count = preferences.getInt("d6Count", 1); //default to 1 d6
  d8Count = preferences.getInt("d8Count", 0);
  d10Count = preferences.getInt("d10Count", 0);
  d12Count = preferences.getInt("d12Count", 0);
  d20Count = preferences.getInt("d20Count", 0);

  //load configuration settings with defaults if not found
  brightness = preferences.getInt("brightness", 255);
  timeToClearDisplay = preferences.getInt("clearTime", 3500);
  stagger = preferences.getInt("stagger", 2);
  frameDelay = preferences.getInt("frameDelay", 0);

  //load roll history with defaults if not found
  size_t historySize = preferences.getBytesLength("rollHistory");
  if (historySize == sizeof(rollHistory)) {
    preferences.getBytes("rollHistory", rollHistory, sizeof(rollHistory));
  } else {
    //initialize empty history if no valid data found
    for (int i = 0; i < 6; i++) {
      for (int j = 0; j < 8; j++) {
        rollHistory[i][j] = -1;
      }
    }
  }
  historyCount = preferences.getInt("historyCount", 0);

  preferences.end();

  //validate total dice count doesn't exceed 8
  int total = getTotalDiceCount();
  if (total > 8) {
    //reset to safe defaults if corrupted data
    coinCount = 0;
    d4Count = 0;
    d6Count = 1;
    d8Count = 0;
    d10Count = 0;
    d12Count = 0;
    d20Count = 0;
  }

  //validate configuration ranges
  if (brightness < 64)
    brightness = 255;
  if (brightness > 255)
    brightness = 255;
  if (timeToClearDisplay < 1000)
    timeToClearDisplay = 3500;
  if (timeToClearDisplay > 10000)
    timeToClearDisplay = 3500;
  if (stagger < 0)
    stagger = 2;
  if (stagger > 20)
    stagger = 2;
  if (frameDelay < 0)
    frameDelay = 0;
  if (frameDelay > 100)
    frameDelay = 0;
}

//custom direct I2C clearing function to avoid flash from display.clearDisplay
void nonGlitchyDisplayClear() {
  for (uint8_t page = 0; page < 8; page++) {
    //set page and column address
    Wire.beginTransmission(0x3C);
    Wire.write(0x00);              //command mode
    Wire.write(0xB0 + page);       //set page address
    Wire.write(0x02);              //lower column address (start at 2)
    Wire.write(0x10);              //higher column address  
    Wire.endTransmission();

    //write the 132 columns in small chunks to avoid I2C buffer overflow
    //that was producing small glitch while clearing
    for (int startCol = 0; startCol < 132; startCol += 16) {
      Wire.beginTransmission(0x3C);
      Wire.write(0x40);            //data mode
      
      int endCol = min(startCol + 16, 132);
      for (int col = startCol; col < endCol; col++) {
        Wire.write(0x00);          //clear pixel(write black to it)
      }
      
      Wire.endTransmission();
    }
  }
  delay(10);
}
