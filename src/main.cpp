#include "config.h"
#include "lsm6ds3_acce.h"
#include "menu.h"
#include "rollDice.h"
#include "utils.h"
#include "welcome_bitmap.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Wire.h>
#include <driver/gpio.h>
#include <esp_sleep.h>

Adafruit_SH1106 display(-1);

// timing variables for sleep and debounce

unsigned long lastButtonPress = 0;
bool buttonPressed = false;

// function prototypes
void showWelcomeMessage();
void goToDeepSleep();
void handleWakeFromButton();

void setup() {

  gpio_hold_dis((gpio_num_t)DISPLAY_POWER_PIN); // disable hold on GPIO 4
  gpio_deep_sleep_hold_dis(); // disable deep sleep hold function


  // lower CPU frequency
  setCpuFrequencyMhz(80);

  // configure display pin
  pinMode(DISPLAY_POWER_PIN, OUTPUT);
  // turn display on
  digitalWrite(DISPLAY_POWER_PIN, HIGH);
  delay(50);

  // configure button with pullup resistor enabled
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // seeds random number generator
  randomSeed(esp_random());
  // loads configuration from flash memory
  loadConfiguration();

  // configure gpio wakeup for esp32-c3 deep sleep mode
  uint64_t wakeup_pin_mask = getAccelerometer() == 1 ? (1ULL << BUTTON_PIN) | (1ULL << ACCE_INT_PIN) : (1ULL << BUTTON_PIN);
  esp_deep_sleep_enable_gpio_wakeup(wakeup_pin_mask, ESP_GPIO_WAKEUP_GPIO_LOW);

  // initializes cleared display
  Wire.begin(SDA_PIN, SCL_PIN);

  nonGlitchyDisplayClear();
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);

  // initializes acce
  bool motionSensorAvailable = initLSM6DS3();

  // check wake up reason
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  // set saved brightness
  setDisplayBrightness(getBrightness());

  // checks what caused the esp32 to wake up
  if (wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {

    handleWakeFromButton();

    goToDeepSleep();

  } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {

    // clear the screen
    nonGlitchyDisplayClear();
    // turns it off completely, controller included
    digitalWrite(DISPLAY_POWER_PIN, LOW);
    // and holds the pin low via RTC
    gpio_hold_en((gpio_num_t)DISPLAY_POWER_PIN);
    gpio_deep_sleep_hold_en();
    goToDeepSleep();

  } else {

    showWelcomeMessage();
    delay(1500);
    nonGlitchyDisplayClear();
    // turns it off completely, controller included
    digitalWrite(DISPLAY_POWER_PIN, LOW);
    // and holds the pin low via RTC
    gpio_hold_en((gpio_num_t)DISPLAY_POWER_PIN);
    gpio_deep_sleep_hold_en();
    goToDeepSleep();
  }
}

void loop() {}

void showWelcomeMessage() {

  int frameDelay = 200;
  int frame = 0;
  unsigned long startTime = millis();

  while (millis() - startTime < 4000) { // animate for 3 seconds
    display.clearDisplay();
    display.drawBitmap(0, 4, greyhound_allArray[frame], 128, 40, WHITE);
    display.drawBitmap(0, 44, welcome_image_dice_allArray[0], 128, 16, WHITE);
    display.display();
    delay(frameDelay);

    frame = (frame + 1) % 3; // cycles frames
  }
}

void goToDeepSleep() {

  // enters deep sleep mode - usb disconnects here
  esp_deep_sleep_start();
}

void handleWakeFromButton() {
  unsigned long buttonHoldStart = millis();
  bool longPressHandled = false;

  if (digitalRead(BUTTON_PIN) == HIGH && getAccelerometer() == 1) {
    // disable accelerometer interrupt during dice rolling
    gpio_intr_disable((gpio_num_t)ACCE_INT_PIN);

    rollDice();

    // Re-enable
    gpio_intr_enable((gpio_num_t)ACCE_INT_PIN);
    
    esp_sleep_enable_timer_wakeup(getTimeToClearDisplay() * 1000);
    gpio_hold_en((gpio_num_t)DISPLAY_POWER_PIN);
    gpio_deep_sleep_hold_en();
    return;
  }

  // Check hold duration while button is still pressed
  while (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long holdDuration = millis() - buttonHoldStart;

    // Open menu immediately when threshold reached
    if (holdDuration >= LONG_PRESS_MS_ENTER_MENU && !longPressHandled) {
      openMenu();
      longPressHandled = true;
      // clear display when exiting menu
      nonGlitchyDisplayClear();
      // turns it off completely, controller included
      digitalWrite(DISPLAY_POWER_PIN, LOW);
      // and holds the pin low via RTC
      gpio_hold_en((gpio_num_t)DISPLAY_POWER_PIN);
      gpio_deep_sleep_hold_en();
      goToDeepSleep();
    }
  }

  // If button was released before long press threshold
  if (!longPressHandled) {
    // enables timer only when dice has been rolled
    esp_sleep_enable_timer_wakeup(getTimeToClearDisplay() * 1000);

    rollDice();
    gpio_hold_en((gpio_num_t)DISPLAY_POWER_PIN); // enable hold on GPIO 4
    gpio_deep_sleep_hold_en(); // enable hold function during deep sleep
    goToDeepSleep();
  }
}
