//minimal OLED + button test


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define SDA_PIN 8
#define SCL_PIN 9
#define BUTTON_PIN 7

Adafruit_SH1106 display(-1);
int count = 0;

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Ready");
  display.display();
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {         //button pressed
    count++;
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Count: ");
    display.println(count);
    display.display();
    delay(200);                        //simplest debounce
  }
}
