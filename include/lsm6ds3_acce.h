#ifndef LSM6DS3_ACCE_H
#define LSM6DS3_ACCE_H

#include <Arduino.h>

//motion interrupt pin definition
#define MOTION_INT_PIN 2  //gpio2 on esp32-c3 for int1 signal

//function prototypes
bool initLSM6DS3();
bool isLSM6DS3Connected();
void configureLSM6DS3ForSleep();
void clearLSM6DS3Interrupts();
void readLSM6DS3Accelerometer(int16_t &ax, int16_t &ay, int16_t &az);
void disableLSM6DS3();

#endif //LSM6DS3_ACCE_H