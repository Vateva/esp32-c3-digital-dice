/*
 * multiple roll deep sleep digital dice
 * allows multiple rolls, sleeps after 5s inactivity
 */

#include "d6_bitmaps.h"
#include "menu.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Wire.h>
#include <driver/gpio.h>
#include <esp_sleep.h>

#define SDA_PIN 8
#define SCL_PIN 9
extern const int BUTTON_PIN =
    3; // gpio3 - only gpio 0-5 can wake from deep sleep
#define DEBOUNCE_DELAY_MS 250
#define TIME_TO_CLEAR_DISPLAY 3500000
#define LONG_PRESS_MS 1000 // milliseconds to consider a long press
#define FRAME_DELAY 0

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

void setup() {
  // configures button with pullup resistor enabled
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // configures gpio wakeup for esp32-c3 deep sleep mode
  esp_deep_sleep_enable_gpio_wakeup(1ULL << BUTTON_PIN,
                                    ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_sleep_enable_timer_wakeup(TIME_TO_CLEAR_DISPLAY);

  // initializes i2c and oled display
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);

  // seeds random number generator with boot time
  // seeds random number generator with analog noise for better entropy
  randomSeed(analogRead(A0) + esp_timer_get_time() + esp_random());

  // check wake up reason
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  // checks what caused the esp32 to wake up
  if (wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {

    /*initConfig();
    esp_sleep_enable_timer_wakeup(currentConfig.sleepTime * 1000000);*/

    handleWakeFromButton();
    goToDeepSleep();

  } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    // woke up from timer - clear display and go back to sleep
    display.clearDisplay();
    display.display();
    delay(10); // ensure i2c transaction completes before sleep
    goToDeepSleep();
  } else {
    // device powered on normally or reset

    /*initConfig();
    esp_sleep_enable_timer_wakeup(currentConfig.sleepTime * 1000000);*/

    showWelcomeMessage();
    delay(2000);
    goToDeepSleep();
  }
}

void loop() {}

void rollDice() {
  // generates random number between 1 and 6 inclusive
  int diceResult = random(1, 7);
  // plays animation common to all results
  for (int i = 0; i < 11; i++) {
    display.clearDisplay();
    display.drawBitmap(30, 0, bitmap_allArray[i], 64, 64,
                       WHITE); // centered at x=30
    display.display();
    delay(FRAME_DELAY); // 50ms per frame
  }
  // plays the corresponding animation
  switch (diceResult) {
  case 1:
    for (int i = 11; i < 18; i++) {
      display.clearDisplay();
      display.drawBitmap(30, 0, bitmap_allArray[i], 64, 64,
                         WHITE); // centered at x=30
      display.display();
      delay(FRAME_DELAY); // 50ms per frame
    }
    break;

  case 2:
    for (int i = 18; i < 25; i++) {
      display.clearDisplay();
      display.drawBitmap(30, 0, bitmap_allArray[i], 64, 64,
                         WHITE); // centered at x=30
      display.display();
      delay(FRAME_DELAY); // 50ms per frame
    }
    break;

  case 3:
    for (int i = 25; i < 32; i++) {
      display.clearDisplay();
      display.drawBitmap(30, 0, bitmap_allArray[i], 64, 64,
                         WHITE); // centered at x=30
      display.display();
      delay(FRAME_DELAY); // 50ms per frame
    }
    break;

  case 4:
    for (int i = 32; i < 39; i++) {
      display.clearDisplay();
      display.drawBitmap(30, 0, bitmap_allArray[i], 64, 64,
                         WHITE); // centered at x=30
      display.display();
      delay(FRAME_DELAY); // 50ms per frame
    }
    break;

  case 5:
    for (int i = 39; i < 46; i++) {
      display.clearDisplay();
      display.drawBitmap(30, 0, bitmap_allArray[i], 64, 64,
                         WHITE); // centered at x=30
      display.display();
      delay(FRAME_DELAY); // 50ms per frame
    }
    break;

  case 6:
    for (int i = 46; i < 53; i++) {
      display.clearDisplay();
      display.drawBitmap(30, 0, bitmap_allArray[i], 64, 64,
                         WHITE); // centered at x=30
      display.display();
      delay(FRAME_DELAY); // 50ms per frame
    }
    break;
  default:
    break;
  }

  // updates physical display with new content
  display.display();

}

void showWelcomeMessage() {
  // clears display and shows initial instructions
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("press to roll dice");
  display.setCursor(0, 20);
  display.println("sleeps after 5s");
  display.setCursor(0, 40);
  display.display();
}

void goToDeepSleep() {

  /* //clears display to save power during sleep
   display.clearDisplay();
   display.display();*/

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
      
      // Wait for button release before sleeping
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
      break;
    }
    delay(10);
  }
  
  // If button was released before long press threshold
  if (!longPressHandled) {
    rollDice();
  }
  
  delay(50);
  goToDeepSleep();
}