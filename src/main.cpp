#include "menu.h"
#include "rollDice.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Wire.h>
#include <driver/gpio.h>
#include <esp_sleep.h>
#include <Fonts/TomThumb.h>

#define SDA_PIN 8
#define SCL_PIN 9
extern const int BUTTON_PIN =
    3; // gpio3 - only gpio 0-5 can wake from deep sleep
#define LONG_PRESS_MS 500 // milliseconds to consider a long press

Adafruit_SH1106 display(-1);

// timing variables for sleep and debounce

unsigned long lastButtonPress = 0;
bool buttonPressed = false;

// function prototypes
void rollDice();
void showWelcomeMessage();
void goToDeepSleep();
void handleWakeFromButton();
void openMenu();
void nonGlitchyDisplayClear();
void turnDisplayOff();

void setup() {
  // configures button with pullup resistor enabled
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // configures gpio wakeup for esp32-c3 deep sleep mode
  esp_deep_sleep_enable_gpio_wakeup(1ULL << BUTTON_PIN,
                                    ESP_GPIO_WAKEUP_GPIO_LOW);

  // seeds random number generator with analog noise for better entropy
  randomSeed(analogRead(A0) + esp_timer_get_time() + esp_random());

  //loads configuration from flash memory
  loadConfiguration();
  
  // initializes cleared display
  Wire.begin(SDA_PIN, SCL_PIN);
  nonGlitchyDisplayClear();
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);

  // check wake up reason
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  // checks what caused the esp32 to wake up
  if (wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {

    /*initConfig();
    esp_sleep_enable_timer_wakeup(currentConfig.sleepTime * 1000000);*/

    handleWakeFromButton();
    goToDeepSleep();

  } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {

    turnDisplayOff();
    nonGlitchyDisplayClear();
    goToDeepSleep();

  } else {
    // device powered on normally or reset

    /*initConfig();
    esp_sleep_enable_timer_wakeup(currentConfig.sleepTime * 1000000);*/

    showWelcomeMessage();
    goToDeepSleep();
  }
}

void loop() {}

void showWelcomeMessage() {
  // clears display and shows initial instructions
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setFont(&TomThumb);
  display.setCursor(0, 0);
  display.println("press to roll dice");
  display.setCursor(0, 20);
  display.println("D20:XX,D20:XX,D20:XX,D20:XX,D20:XX,D20:XX,D20:XX,D20:XX");
  display.setCursor(0, 40);
  display.display();
}

void goToDeepSleep() {

  // enters deep sleep mode - usb disconnects here
  esp_deep_sleep_start();
}
void handleWakeFromButton() {
  unsigned long buttonHoldStart = millis();
  bool longPressHandled = false;

  // Check hold duration while button is still pressed
  while (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long holdDuration = millis() - buttonHoldStart;

    // Open menu immediately when threshold reached
    if (holdDuration >= LONG_PRESS_MS && !longPressHandled) {
      openMenu();
      longPressHandled = true;
    }
  }

  // If button was released before long press threshold
  if (!longPressHandled) {
    esp_sleep_enable_timer_wakeup(getClearDisplayTime()*1000);
    rollDice();
  }
}
// clears screen via I2C to avoid flash/glitch while clearing
void nonGlitchyDisplayClear() {

  // clear all display memory by sending black pixels directly
  for (int page = 0; page < 8; page++) {
    // set page address (0-7 for 64 pixel height ÷ 8)
    Wire.beginTransmission(0x3C);
    Wire.write(0x00);        // command mode
    Wire.write(0xB0 + page); // set page address
    Wire.write(0x02);        // set column start low (sh1106 offset)
    Wire.write(0x10);        // set column start high
    Wire.endTransmission();

    // send 128 bytes of black pixels (0x00) for this page
    Wire.beginTransmission(0x3C);
    Wire.write(0x40); // data mode
    for (int col = 0; col < 128; col++) {
      Wire.write(0x00); // black pixel data
    }
    Wire.endTransmission();
  }
}
// turns the screen on
void turnDisplayOff() {
  // turns on display using direct i2c command
  Wire.beginTransmission(0x3C);
  Wire.write(0x00); // command mode
  Wire.write(0xAE); // display OFF command
  Wire.endTransmission();
}
