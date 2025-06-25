#include "menu.h"
#include "config.h"
#include "utils.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Fonts/TomThumb.h>
#include <Preferences.h>
#include <Wire.h>

// initialize rtc memory struct so it loads from flash on boot
RTC_DATA_ATTR RtcConfig rtcConfig = {false};

// external references to main.cpp objects
extern Adafruit_SH1106 display;
extern unsigned long lastButtonPress;
extern bool buttonPressed;
int rollHistory[6][8];
int historyCount;

// preferences object for flash storage
Preferences preferences;

// main menu option labels
const char *mainMenuItems[MAIN_MENU_ITEMS] = {"Dice", "Roll history",
                                              "Configuration", "Exit"};
// dice menu option labels
const char *diceMenuItems[DICE_MENU_ITEMS] = {"Coin", "D4",  "D6",  "D8",
                                              "D10",  "D12", "D20", "Back"};
// config menu option labels
const char *configMenuItems[CONFIG_MENU_ITEMS] = {
    "Brightness", "Time to clear", "Stagger time", "Accelerometer", "Back"};

// dice count array
int diceCount[7] = {COIN_COUNT_DEFAULT, D4_COUNT_DEFAULT,  D6_COUNT_DEFAULT,
                    D8_COUNT_DEFAULT,   D10_COUNT_DEFAULT, D12_COUNT_DEFAULT,
                    D20_COUNT_DEFAULT}; // coin, d4, d6, d8, d10, d12, d20

// default configuration values
int brightness = BRIGHTNESS_DEFAULT;
int timeToClearDisplay = TIME_TO_CLEAR_DISPLAY_DEFAULT; // milliseconds
int stagger = STAGGER_DEFAULT;
int accelerometer = ACCELEROMETER_DEFAULT;

// timing variables for button handling
unsigned long buttonPressStart = 0;
bool longPressDetected = false;
// menu navigation variables
int currentMenu = MENU_MAIN;
int currentSelection = 0;
bool menuExitRequested = false;
bool changedConfig = false;

// function prototypes
void drawMenu();
void handleButtonPress();
bool isButtonPressed();
void executeMenuAction();
int getTotalDiceCount();
int getMaxAllowedCount(int currentTypeCount);
void saveConfiguration();
void loadConfiguration();

void openMenu() {
  // loads configuration from flash memory
  loadConfiguration();

  // resets menu state when opening
  currentMenu = MENU_MAIN;
  currentSelection = 0;
  buttonPressed = true;
  longPressDetected = true;
  lastButtonPress = millis();
  buttonPressStart = 0;
  menuExitRequested = false;
  changedConfig = false;

  // draws initial menu
  drawMenu();

  // main menu loop - stays here until exit is selected
  while (!menuExitRequested) {
    handleButtonPress();
    delay(10); // small delay to prevent excessive cpu usage
  }

  // saves configuration to flash memorybefore exiting if something changed
  if (changedConfig) {
    saveConfiguration();
  }
}

void drawMenu() {
  // clears display for fresh menu render
  display.clearDisplay();

  // sets text properties for menu display
  display.setTextColor(WHITE);
  display.setTextSize(1);

  switch (currentMenu) {
  case MENU_MAIN:
    // draws menu title at top of screen
    display.setCursor(37, 5);
    display.println("Main menu");

    // draws each menu item with selection indicator
    for (int i = 0; i < MAIN_MENU_ITEMS; i++) {
      display.setCursor(2,
                        15 + (i * 13)); // positions each item 12 pixels apart

      // draws selection indicator for current item
      if (i == currentSelection) {
        display.print(">");
      } else {
        display.print(" "); // spaces for alignment when not selected
      }

      // draws the menu item text
      display.print(mainMenuItems[i]);
    }
    break;
  case MENU_HISTORY:
    // draws roll history menu title
    display.setCursor(15, 5);
    display.println("  Roll History");

    // checks if there are any rolls to display
    if (historyCount == 0) {
      // shows message when no history exists
      display.setCursor(20, 25);
      display.println("No rolls yet");
    } else {
      // switches to small font for history entries
      display.setFont(&TomThumb);

      // determines how many entries to show (max 6)
      int entriesToShow = (historyCount > 6) ? 6 : historyCount;

      // draws each history entry
      for (int i = 0; i < entriesToShow; i++) {
        display.setCursor(0,
                          19 + (i * 7)); // positions each entry 7 pixels apart

        // formats entry label based on age
        if (i == 0) {
          display.print("Last: ");
        } else {
          display.print(" n-");
          display.print(i + 1);
          display.print(" : ");
        }

        // gets the roll to display (newest first)
        int rollIndex = i;

        // counts non-minus one dice in this roll
        int diceCount = 0;
        for (int j = 0; j < 8; j++) {
          if (rollHistory[rollIndex][j] != -1) {
            diceCount++;
          }
        }

        // displays dice values separated by commas
        for (int j = 0; j < diceCount; j++) {
          display.print(rollHistory[rollIndex][j]);
          if (j < diceCount - 1) {
            display.print("-");
          }
        }
      }

      // switches back to default font for back button
      display.setFont();
    }

    // draws back button with selection indicator
    display.setCursor(2, 56);
    if (currentSelection == 0) {
      display.print(">");
    } else {
      display.print(" ");
    }
    display.print("Back");
    break;

  case MENU_DICE:
    // draws dice menu title
    display.setCursor(37, 5);
    display.println("Dice Menu");

    // draws dice menu items in two columns with counts
    for (int i = 0; i < DICE_MENU_ITEMS; i++) {
      int col = (i / 4); // 0 for left column, 1 for right column
      int row = i % 4;   // row within column (0-3)

      display.setCursor(2 + (col * 64),
                        17 + (row * 13)); // 64 pixels apart for columns

      // draws selection indicator for current item
      if (i == currentSelection) {
        display.print(">");
      } else {
        display.print(" ");
      }

      // draws the menu item text
      display.print(diceMenuItems[i]);

      // show current count for each dice type (not for "Back")
      if (i < DICE_MENU_ITEMS - 1) {
        display.print(":");
        switch (i) {
        case 0: // coin
          display.print(diceCount[0]);
          break;
        case 1: // d4
          display.print(diceCount[1]);
          break;
        case 2: // d6
          display.print(diceCount[2]);
          break;
        case 3: // d8
          display.print(diceCount[3]);
          break;
        case 4: // d10
          display.print(diceCount[4]);
          break;
        case 5: // d12
          display.print(diceCount[5]);
          break;
        case 6: // d20
          display.print(diceCount[6]);
          break;
        }
      }
    }
    break;

  case MENU_CONFIG:
    // draws configuration menu title
    display.setCursor(25, 5);
    display.println("Configuration");

    // draws config menu items with selection indicator
    for (int i = 0; i < CONFIG_MENU_ITEMS; i++) {
      display.setCursor(2, 16 + (i * 10)); // positions each item

      // draws selection indicator for current item
      if (i == currentSelection) {
        display.print(">");
      } else {
        display.print(" ");
      }

      // draws the menu item text
      display.print(configMenuItems[i]);

      // show current values for config items (not for "Back")
      if (i < CONFIG_MENU_ITEMS - 1) {
        display.print(":");
        switch (i) {
        case 0: // brightness
          display.print(map(brightness, 0, 255, 0, 100));
          display.print("%");
          break;
        case 1: // clear display time
          display.print(timeToClearDisplay / 1000.0, 1);
          display.print("s");
          break;
        case 2: // animation frames
          display.print(stagger);
          break;
        case 3: // accelerometer
          if (accelerometer == 1) {
            display.print("On");
          } else if (accelerometer == 0){
            display.print("Off");
          }
          break;
        }
      }
    }
    break;
  }

  // updates physical display with menu content
  display.display();
}

void handleButtonPress() {
  bool currentButtonState = isButtonPressed();
  unsigned long currentTime = millis();

  // detects button press start
  if (currentButtonState && !buttonPressed) {
    // checks debounce timing to avoid false triggers
    if (currentTime - lastButtonPress > DEBOUNCE_DELAY_MS) {
      buttonPressed = true;
      buttonPressStart = currentTime;
      longPressDetected = false;
    }
  }

  // detects long press while button is held
  if (buttonPressed && currentButtonState) {
    if (currentTime - buttonPressStart >= LONG_PRESS_MENU_SELECTION &&
        !longPressDetected) {
      longPressDetected = true;
      executeMenuAction();
    }
  }

  // detects button release for navigation
  if (!currentButtonState && buttonPressed) {
    buttonPressed = false;
    lastButtonPress = currentTime;

    // only navigates if it wasn't a long press selection
    if (!longPressDetected) {
      // moves to next menu item based on current menu
      switch (currentMenu) {
      case MENU_MAIN:
        currentSelection = (currentSelection + 1) % MAIN_MENU_ITEMS;
        break;
      // no case for MENU_HISTORY since no navigation
      case MENU_DICE:
        currentSelection = (currentSelection + 1) % DICE_MENU_ITEMS;
        break;
      case MENU_CONFIG:
        currentSelection = (currentSelection + 1) % CONFIG_MENU_ITEMS;
        break;
      }

      // redraws menu with new selection
      drawMenu();
    }
  }
}

// calculates total number of dice across all types
int getTotalDiceCount() {
  return diceCount[0] + diceCount[1] + diceCount[2] + diceCount[3] +
         diceCount[4] + diceCount[5] + diceCount[6];
}

// calculates maximum allowed count for cycling a specific dice type
int getMaxAllowedCount(int currentTypeCount) {
  int totalOthers = getTotalDiceCount() - currentTypeCount;
  return 8 - totalOthers;
}

void executeMenuAction() {
  switch (currentMenu) {
  case MENU_MAIN:
    switch (currentSelection) {
    case 0: // dice menu
      currentMenu = MENU_DICE;
      currentSelection = 0;
      drawMenu();
      break;
    case 1: // configuration menu
      currentMenu = MENU_HISTORY;
      currentSelection = 0;
      drawMenu();
      break;
    case 2: // configuration menu
      currentMenu = MENU_CONFIG;
      currentSelection = 0;
      drawMenu();
      break;
    case 3: // exit
      menuExitRequested = true;
      break;
    }
    break;

  case MENU_DICE:
    // handles dice count cycling for all dice types
    if (currentSelection < DICE_MENU_ITEMS - 1) {
      int maxAllowed = getMaxAllowedCount(diceCount[currentSelection]);
      diceCount[currentSelection] =
          (diceCount[currentSelection] + 1) % (maxAllowed + 1);
      changedConfig = true;
      drawMenu();
    } else { // back option
      currentMenu = MENU_MAIN;
      currentSelection = 0;
      drawMenu();
    }
    break;
  case MENU_HISTORY:
    // back button (only option)
    currentMenu = MENU_MAIN;
    currentSelection = 1; // return to "Roll history" option
    drawMenu();
    break;
  case MENU_CONFIG:
    switch (currentSelection) {
    case 0: // brightness
      // cycle through brightness levels
      if (brightness == 192) {
        brightness = 255; // force 100% after 75%
      } else if (brightness == 255) {
        brightness = 64; // wrap back to 25% after 100%
      } else {
        brightness = brightness + 64; // normal increment
      }
      // apply brightness to display
      setDisplayBrightness(brightness);
      changedConfig = true; // flag to save to flash
      drawMenu();
      break;
    case 1: // time to clear
      // cycle through times (s)
      timeToClearDisplay =
          (timeToClearDisplay >= 5000) ? 1000 : timeToClearDisplay + 500;
      changedConfig = true; // flag to save to flash
      drawMenu();
      break;
    case 2: // stagger time
      // cycle through stagger time
      stagger = (stagger == 5) ? 0 : stagger + 1;
      changedConfig = true; // flag to save to flash
      drawMenu();
      break;
    case 3: // accelerometer 1/0 on/off
      accelerometer = (accelerometer == 1) ? 0 : 1;
      changedConfig = true; // flag to save to flash
      drawMenu();
      break;
    case 4: // back
      currentMenu = MENU_MAIN;
      currentSelection = 1; // return to configuration option
      drawMenu();
      break;
    }
    break;
  }
}

bool isButtonPressed() {
  // reads button state - returns true when button is pressed
  // button is active low due to pullup resistor configuration
  return digitalRead(BUTTON_PIN) == LOW;
}

// functions to access menu settings from main.cpp and rolldice.cpp

int getBrightness() { return brightness; }

int getTimeToClearDisplay() { return timeToClearDisplay; }

int getStagger() { return stagger; }

int getAccelerometer() { return accelerometer; }

// new getter functions for individual dice counts
int getCoinCount() { return diceCount[0]; }
int getD4Count() { return diceCount[1]; }
int getD6Count() { return diceCount[2]; }
int getD8Count() { return diceCount[3]; }
int getD10Count() { return diceCount[4]; }
int getD12Count() { return diceCount[5]; }
int getD20Count() { return diceCount[6]; }

// saves all configuration
void saveConfiguration() {
  preferences.begin("diceConfig", false); // false = read/write mode

  // save dice counts
  preferences.putInt("coinCount", diceCount[0]);
  preferences.putInt("d4Count", diceCount[1]);
  preferences.putInt("d6Count", diceCount[2]);
  preferences.putInt("d8Count", diceCount[3]);
  preferences.putInt("d10Count", diceCount[4]);
  preferences.putInt("d12Count", diceCount[5]);
  preferences.putInt("d20Count", diceCount[6]);

  // save configuration settings
  preferences.putInt("brightness", brightness);
  preferences.putInt("clearTime", timeToClearDisplay);
  preferences.putInt("stagger", stagger);
  preferences.putInt("accelerometer", accelerometer);

  // save roll history
  preferences.putBytes("rollHistory", rollHistory, sizeof(rollHistory));
  preferences.putInt("historyCount", historyCount);

  // update rtc memory with new values
  rtcConfig.coinCount = diceCount[0];
  rtcConfig.d4Count = diceCount[1];
  rtcConfig.d6Count = diceCount[2];
  rtcConfig.d8Count = diceCount[3];
  rtcConfig.d10Count = diceCount[4];
  rtcConfig.d12Count = diceCount[5];
  rtcConfig.d20Count = diceCount[6];
  rtcConfig.brightness = brightness;
  rtcConfig.timeToClearDisplay = timeToClearDisplay;
  rtcConfig.stagger = stagger;
  rtcConfig.accelerometer = accelerometer;
  rtcConfig.historyCount = historyCount;

  // copy roll history to rtc
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 8; j++) {
      rtcConfig.rollHistory[i][j] = rollHistory[i][j];
    }
  }

  rtcConfig.isValid = true;

  preferences.end();
}

// loads all configuration from flash memory
void loadConfiguration() {
  // check if this is cold boot or deep sleep wake
  esp_reset_reason_t reset_reason = esp_reset_reason();
  bool isColdBoot =
      (reset_reason == ESP_RST_POWERON || reset_reason == ESP_RST_BROWNOUT ||
       reset_reason == ESP_RST_SW || reset_reason == ESP_RST_EXT);

  if (!isColdBoot && rtcConfig.isValid) {
    // deep sleep wake - use rtc memory (fast, no flash access)
    diceCount[0] = rtcConfig.coinCount;
    diceCount[1] = rtcConfig.d4Count;
    diceCount[2] = rtcConfig.d6Count;
    diceCount[3] = rtcConfig.d8Count;
    diceCount[4] = rtcConfig.d10Count;
    diceCount[5] = rtcConfig.d12Count;
    diceCount[6] = rtcConfig.d20Count;
    brightness = rtcConfig.brightness;
    timeToClearDisplay = rtcConfig.timeToClearDisplay;
    stagger = rtcConfig.stagger;
    accelerometer = rtcConfig.accelerometer;
    historyCount = rtcConfig.historyCount;

    // copy roll history from rtc
    for (int i = 0; i < 6; i++) {
      for (int j = 0; j < 8; j++) {
        rollHistory[i][j] = rtcConfig.rollHistory[i][j];
      }
    }

    return; // skip flash access entirely
  }

  // cold boot or invalid rtc - load from flash
  preferences.begin("diceConfig", true); // true = read-only mode

  // load all values from flash
  diceCount[0] = preferences.getInt("coinCount", COIN_COUNT_DEFAULT);
  diceCount[1] = preferences.getInt("d4Count", D4_COUNT_DEFAULT);
  diceCount[2] = preferences.getInt("d6Count", D6_COUNT_DEFAULT);
  diceCount[3] = preferences.getInt("d8Count", D8_COUNT_DEFAULT);
  diceCount[4] = preferences.getInt("d10Count", D10_COUNT_DEFAULT);
  diceCount[5] = preferences.getInt("d12Count", D12_COUNT_DEFAULT);
  diceCount[6] = preferences.getInt("d20Count", D20_COUNT_DEFAULT);
  brightness = preferences.getInt("brightness", BRIGHTNESS_DEFAULT);
  timeToClearDisplay =
      preferences.getInt("clearTime", TIME_TO_CLEAR_DISPLAY_DEFAULT);
  stagger = preferences.getInt("stagger", STAGGER_DEFAULT);
  accelerometer = preferences.getInt("accelerometer", ACCELEROMETER_DEFAULT);
  historyCount = preferences.getInt("historyCount", 0);

  // load roll history
  size_t historySize = preferences.getBytesLength("rollHistory");
  if (historySize == sizeof(rollHistory)) {
    preferences.getBytes("rollHistory", rollHistory, sizeof(rollHistory));
  } else {
    // initialize empty history if no valid data found
    for (int i = 0; i < 6; i++) {
      for (int j = 0; j < 8; j++) {
        rollHistory[i][j] = -1;
      }
    }
  }

  preferences.end();

  // single validation check - if any value is out of range, reset all to
  // defaults
  if (diceCount[0] < 0 || diceCount[0] > 8 || diceCount[1] < 0 ||
      diceCount[1] > 8 || diceCount[2] < 0 || diceCount[2] > 8 ||
      diceCount[3] < 0 || diceCount[3] > 8 || diceCount[4] < 0 ||
      diceCount[4] > 8 || diceCount[5] < 0 || diceCount[5] > 8 ||
      diceCount[6] < 0 || diceCount[6] > 8 || getTotalDiceCount() > 8 ||
      brightness < 64 || brightness > 255 || timeToClearDisplay < 1000 ||
      timeToClearDisplay > 10000 || stagger < 0 || stagger > 20 ||
      accelerometer < 0 || accelerometer > 1 || historyCount < 0 ||
      historyCount > 6) {

    // reset all configuration to defaults using defined constants
    diceCount[0] = COIN_COUNT_DEFAULT;
    diceCount[1] = D4_COUNT_DEFAULT;
    diceCount[2] = D6_COUNT_DEFAULT;
    diceCount[3] = D8_COUNT_DEFAULT;
    diceCount[4] = D10_COUNT_DEFAULT;
    diceCount[5] = D12_COUNT_DEFAULT;
    diceCount[6] = D20_COUNT_DEFAULT;
    brightness = BRIGHTNESS_DEFAULT;
    timeToClearDisplay = TIME_TO_CLEAR_DISPLAY_DEFAULT;
    stagger = STAGGER_DEFAULT;
    accelerometer = ACCELEROMETER_DEFAULT;
    historyCount = 0;

    // clear roll history
    for (int i = 0; i < 6; i++) {
      for (int j = 0; j < 8; j++) {
        rollHistory[i][j] = -1;
      }
    }
  }

  // after loading from flash, update rtc memory
  rtcConfig.coinCount = diceCount[0];
  rtcConfig.d4Count = diceCount[1];
  rtcConfig.d6Count = diceCount[2];
  rtcConfig.d8Count = diceCount[3];
  rtcConfig.d10Count = diceCount[4];
  rtcConfig.d12Count = diceCount[5];
  rtcConfig.d20Count = diceCount[6];
  rtcConfig.brightness = brightness;
  rtcConfig.timeToClearDisplay = timeToClearDisplay;
  rtcConfig.stagger = stagger;
  rtcConfig.accelerometer = accelerometer;
  rtcConfig.historyCount = historyCount;

  // copy roll history to rtc
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 8; j++) {
      rtcConfig.rollHistory[i][j] = rollHistory[i][j];
    }
  }

  rtcConfig.isValid = true; // mark rtc data as valid
}