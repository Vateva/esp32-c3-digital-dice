#include "utils.h"
#include <Wire.h>
#include <Adafruit_SH1106.h>
//external reference to display object from main.cpp
extern Adafruit_SH1106 display;

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

//custom direct I2C clearing function to avoid flash from display.clearDisplay
void nonGlitchyDisplayClear() {
  for (uint8_t page = 0; page < 8; page++) {
    //set page and column address
    Wire.beginTransmission(0x3C);
    Wire.write(0x00);        //command mode
    Wire.write(0xB0 + page); //set page address
    Wire.write(0x02);        //lower column address (start at 2)
    Wire.write(0x10);        //higher column address
    Wire.endTransmission();

    //write the 132 columns in small chunks to avoid I2C buffer overflow
    //that was producing small glitch while clearing
    for (int startCol = 0; startCol < 132; startCol += 16) {
      Wire.beginTransmission(0x3C);
      Wire.write(0x40); //data mode

      int endCol = min(startCol + 16, 132);
      for (int col = startCol; col < endCol; col++) {
        Wire.write(0x00); //clear pixel(write black to it)
      }

      Wire.endTransmission();
    }
  }
  delay(10);
}