/*
 * Multiple Roll Deep Sleep Digital Dice
 * Allows multiple rolls, sleeps after 5s inactivity
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

#define SDA_PIN 8
#define SCL_PIN 9
#define BUTTON_PIN 3  //gpio3 - only gpio 0-5 can wake from deep sleep on esp32-c3
#define INACTIVITY_TIMEOUT_MS 5000
#define DEBOUNCE_DELAY_MS 500

Adafruit_SH1106 display(-1);

//timing variables
unsigned long lastActivityTime = 0;
unsigned long lastButtonPress = 0;
bool buttonPressed = false;

//function prototypes
void rollDice();
void showWelcomeMessage();
void goToDeepSleep();
void checkButton();

void setup() {
  //configure button with pullup
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  //configure gpio wakeup for esp32-c3 deep sleep (only gpio 0-5 work)
  esp_deep_sleep_enable_gpio_wakeup(1ULL << BUTTON_PIN, ESP_GPIO_WAKEUP_GPIO_LOW);
  
  //initialize display
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  
  //seed random with boot time
  randomSeed(esp_timer_get_time());
  
  //check wake up reason
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  if (wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {
    //woke up from button press
    rollDice();
  } else {
    //first boot or reset - show ready message
    showWelcomeMessage();
    delay(2000);
  }
  
  //reset activity timer
  lastActivityTime = millis();
}

void loop() {
  checkButton();
  
  //check for inactivity timeout
  if (millis() - lastActivityTime > INACTIVITY_TIMEOUT_MS) {
    goToDeepSleep();
  }
  
  //update display every second to show countdown
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 1000) {
    lastDisplayUpdate = millis();
  }
}

void checkButton() {
  bool currentButtonState = (digitalRead(BUTTON_PIN) == LOW);
  
  //debounce logic
  if (currentButtonState && !buttonPressed && (millis() - lastButtonPress > DEBOUNCE_DELAY_MS)) {
    buttonPressed = true;
    lastButtonPress = millis();
    lastActivityTime = millis();  //reset inactivity timer
    
    //roll the dice
    rollDice();
  } else if (!currentButtonState && buttonPressed) {
    buttonPressed = false;
  }
}




void rollDice() {
  //generate random number between 1 and 6
  int diceResult = random(1, 7);
  
  //show dice result
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  //large dice number in center
  display.setTextSize(4);
  display.setCursor(50, 20);
  display.print(diceResult);
  
  
  display.display();
}

void showWelcomeMessage() {
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
  //clear display to save power
  display.clearDisplay();
  display.display();

  //enter deep sleep (usb will disconnect here)
  esp_deep_sleep_start();
}