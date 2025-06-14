#include "config.h"
#include "menu.h"
#include "rollDice.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Wire.h>
#include <driver/gpio.h>
#include <esp_sleep.h>

Adafruit_SH1106 display(-1);

//timing variables for sleep and debounce

unsigned long lastButtonPress = 0;
bool buttonPressed = false;

//function prototypes
void showWelcomeMessage();
void goToDeepSleep();
void handleWakeFromButton();

void setup() {

  gpio_hold_dis((gpio_num_t)DISPLAY_POWER_PIN); //disable hold on GPIO 4
  gpio_deep_sleep_hold_dis(); //disable deep sleep hold function

  //lower CPU frequency
  setCpuFrequencyMhz(80);

  //configure display pin
  pinMode(DISPLAY_POWER_PIN, OUTPUT);
  //turn display on
  digitalWrite(DISPLAY_POWER_PIN, HIGH);
  delay(50);

  //configure button with pullup resistor enabled
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  //configure gpio wakeup for esp32-c3 deep sleep mode
  esp_deep_sleep_enable_gpio_wakeup(1ULL << BUTTON_PIN,
                                    ESP_GPIO_WAKEUP_GPIO_LOW);

  //seeds random number generator
  randomSeed(esp_random());
  //loads configuration from flash memory
  loadConfiguration();

  //initializes cleared display
  Wire.begin(SDA_PIN, SCL_PIN);
 
  nonGlitchyDisplayClear();
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
 


  //check wake up reason
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  //checks what caused the esp32 to wake up
  if (wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {

    handleWakeFromButton();
    goToDeepSleep();

  } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {

    //clear the screen
    nonGlitchyDisplayClear();
    //turns it off completely, controller included
    digitalWrite(DISPLAY_POWER_PIN, LOW);
    //and holds the pin low via RTC
    gpio_hold_en((gpio_num_t)DISPLAY_POWER_PIN);
    gpio_deep_sleep_hold_en();
    goToDeepSleep();

  } else {

    showWelcomeMessage();
    //turns it off completely, controller included
    digitalWrite(DISPLAY_POWER_PIN, LOW);
    //and holds the pin low via RTC
    gpio_hold_en((gpio_num_t)DISPLAY_POWER_PIN);
    gpio_deep_sleep_hold_en();
    goToDeepSleep();
  }
}

void loop() {}

void showWelcomeMessage() {
  //clears display and shows initial instructions
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("press to roll dice");
  display.setCursor(0, 20);
  display.println("D20:XX,D20:XX,D20:XX,D20:XX,D20:XX,D20:XX,D20:XX,D20:XX");
  display.setCursor(0, 40);
  display.display();
}

void goToDeepSleep() {

  //enters deep sleep mode - usb disconnects here
  esp_deep_sleep_start();
}
void handleWakeFromButton() {
  unsigned long buttonHoldStart = millis();
  bool longPressHandled = false;

  //Check hold duration while button is still pressed
  while (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long holdDuration = millis() - buttonHoldStart;

    //Open menu immediately when threshold reached
    if (holdDuration >= LONG_PRESS_MS_ENTER_MENU && !longPressHandled) {
      openMenu();
      longPressHandled = true;
      //clear display when exiting menu
      //nonGlitchyDisplayClear();
      //turns it off completely, controller included
      digitalWrite(DISPLAY_POWER_PIN, LOW);

      //and holds the pin low via RTC
      gpio_hold_en((gpio_num_t)DISPLAY_POWER_PIN);
      gpio_deep_sleep_hold_en();
    }
  }

  //If button was released before long press threshold
  if (!longPressHandled) {
    //enables timer only when dice has been rolled
    esp_sleep_enable_timer_wakeup(getTimeToClearDisplay() * 1000);
    rollDice();
    gpio_hold_en((gpio_num_t)DISPLAY_POWER_PIN); //enable hold on GPIO 4
    gpio_deep_sleep_hold_en(); //enable hold function during deep sleep
    goToDeepSleep();
  }
}

