/*
 * menu system - KEEPING ORIGINAL STRUCTURE
 */

#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Wire.h>

// external references to main.cpp objects
extern Adafruit_SH1106 display;
extern unsigned long lastButtonPress;
extern bool buttonPressed;

// button configuration - matches main.cpp setup
#define BUTTON_PIN 3
#define DEBOUNCE_DELAY_MS 100
#define LONG_PRESS_TIME_MS 500

// menu state tracking
#define MENU_MAIN 0
#define MENU_DICE 1
#define MENU_CONFIG 2
int currentMenu = MENU_MAIN;

// menu options count and current selection
#define MAIN_MENU_ITEMS 3
#define DICE_MENU_ITEMS 8
#define CONFIG_MENU_ITEMS 5
int currentSelection = 0;

// main menu option labels
const char *mainMenuItems[MAIN_MENU_ITEMS] = {"Dice", "Configuration", "Exit"};
// dice menu option labels - FIXED: correct array name
const char *diceMenuItems[DICE_MENU_ITEMS] = {"Amount", "D6",  "D20", "D4",
                                              "D8",     "D10", "D12", "Back"};
// config menu option labels
const char *configMenuItems[CONFIG_MENU_ITEMS] = {"Brightness", "Time to clear",
                                                  "Animation frames",
                                                  "Frame delay(ms)", "Back"};

// configuration values
int numberOfDice = 1;
int selectedDiceType = 6; // stores which dice type was selected
int brightness = 255;
int clearDisplayTime = 3500; // milliseconds
int animationFrames = 10;
int frameDelay = 0;

// timing variables for button handling
unsigned long buttonPressStart = 0;
bool longPressDetected = false;
bool menuExitRequested = false;

// function prototypes
void drawMenu();
void handleButtonPress();
bool isButtonPressed();
void executeMenuAction();

void openMenu() {
// resets menu state when opening
  currentMenu = MENU_MAIN;
  currentSelection = 0;
  buttonPressed = true;
  longPressDetected = true;
  lastButtonPress = millis();
  buttonPressStart = 0;
  menuExitRequested = false;

  // draws initial menu
  drawMenu();

  // main menu loop - stays here until exit is selected
  while (!menuExitRequested) {
    handleButtonPress();
    delay(10); // small delay to prevent excessive cpu usage
  }

  // clear display when exiting menu
  display.clearDisplay();
  display.display();
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
    display.setCursor(15, 0);
    display.println("    Main menu");

    // draws each menu item with selection indicator
    for (int i = 0; i < MAIN_MENU_ITEMS; i++) {
      display.setCursor(0,
                        20 + (i * 12)); // positions each item 12 pixels apart

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

  case MENU_DICE:
    // draws dice menu title
    display.setCursor(20, 0);
    display.println("   Dice Menu");

    // draws dice menu items in two columns
    for (int i = 0; i < DICE_MENU_ITEMS; i++) {
      int col = i / 4; // 0 for left column, 1 for right column
      int row = i % 4; // row within column (0-3)

      display.setCursor(col * 64,
                        12 + (row * 12)); // 64 pixels apart for columns

      // draws selection indicator for current item
      if (i == currentSelection) {
        display.print(">");
      } else {
        display.print(" ");
      }

      // draws the menu item text
      display.print(diceMenuItems[i]);

      // show current value for number of dice
      if (i == 0) {
        display.print(":");
        display.print(numberOfDice);
      }
    }
    break;

  case MENU_CONFIG:
    // draws configuration menu title
    display.setCursor(8, 0);
    display.println("  Configuration");

    // draws config menu items with selection indicator
    for (int i = 0; i < CONFIG_MENU_ITEMS; i++) {
      display.setCursor(0, 12 + (i * 10)); // positions each item

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
        case 0: // Brightness
          display.print(map(brightness, 0, 255, 0, 100));
          display.print("%");
          break;
        case 1: // Clear Display Time
          display.print(clearDisplayTime / 1000);
          display.print("s");
          break;
        case 2: // Animation frames
          display.print(animationFrames);
          break;
        case 3: // Animation FPS
          display.print(frameDelay);
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
    if (currentTime - buttonPressStart >= LONG_PRESS_TIME_MS &&
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

// Function to set display brightness/contrast
void setDisplayBrightness(uint8_t brightness) {
  // SH1106 uses contrast command for brightness
  // Send command sequence to set contrast
  Wire.beginTransmission(0x3C); // Display I2C address
  Wire.write(0x00);             // Command mode
  Wire.write(0x81);             // Set contrast command
  Wire.write(brightness);       // Contrast value 0-255
  Wire.endTransmission();
}

void executeMenuAction() {
  switch (currentMenu) {
  case MENU_MAIN:
    switch (currentSelection) {
    case 0: // Dice menu
      currentMenu = MENU_DICE;
      currentSelection = 0;
      drawMenu();
      break;
    case 1: // Configuration menu
      currentMenu = MENU_CONFIG;
      currentSelection = 0;
      drawMenu();
      break;
    case 2: // Exit
      menuExitRequested = true;
      break;
    }
    break;

  case MENU_DICE:
    switch (currentSelection) {
    case 0:                                  // Number of dice
      numberOfDice = (numberOfDice % 2) + 1; // cycles 1-10
      drawMenu();
      break;
    case 1: // D6
      selectedDiceType = 6;
      // Store this for main.cpp to use later
      drawMenu();
      break;
    case 2: // D20
      selectedDiceType = 20;
      drawMenu();
      break;
    case 3: // D4
      selectedDiceType = 4;
      drawMenu();
      break;
    case 4: // D8
      selectedDiceType = 8;
      drawMenu();
      break;
    case 5: // D10
      selectedDiceType = 10;
      drawMenu();
      break;
    case 6: // D12
      selectedDiceType = 12;
      drawMenu();
      break;
    case 7: // Back
      currentMenu = MENU_MAIN;
      currentSelection = 0;
      drawMenu();
      break;
    }
    break;

  case MENU_CONFIG:
    switch (currentSelection) {
    case 0: // Brightness
      // Cycle through brightness levels
      brightness = (brightness + 64) % 256;
      if (brightness == 0)
        brightness = 64; // Min brightness
      // Apply brightness to display
      setDisplayBrightness(brightness);
      drawMenu();
      break;
    case 1: // Time to clear
      // Cycle through times (s)
      clearDisplayTime =
          (clearDisplayTime >= 5000) ? 1000 : clearDisplayTime + 500;
      drawMenu();
      break;
    case 2: // Animation frames
      // Cycle through frame counts: 5, 10, 15, 20
      animationFrames = (animationFrames == 20) ? 5 : animationFrames + 5;
      drawMenu();
      break;
    case 3: // Frame delay(ms)
      frameDelay = (frameDelay == 75) ? 0 : frameDelay + 5;
      drawMenu();
      break;
    case 4: // Back
      currentMenu = MENU_MAIN;
      currentSelection = 1; // return to Configuration option
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

// Functions to access menu settings from main.cpp
int getNumberOfDice() { return numberOfDice; }
int getSelectedDiceType() { return selectedDiceType; }
int getBrightness() { return brightness; }
int getClearDisplayTime() { return clearDisplayTime; }
int getAnimationFrames() { return animationFrames; }
int getframeDelay() { return frameDelay; }