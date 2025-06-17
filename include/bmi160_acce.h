#ifndef BMI160_ACCE_H
#define BMI160_ACCE_H

#include <Arduino.h>

//motion interrupt pin connected to bmi160 int1
#define MOTION_INT_PIN 2

//function prototypes for bmi160 accelerometer control

//initializes bmi160 and configures it for motion detection
//returns true if successful, false if bmi160 not found or error
bool initBMI160();

//configures bmi160 motion detection parameters before going to sleep
//should be called right before entering deep sleep
void configureBMI160ForSleep();

//clears any pending interrupts from bmi160
//should be called after waking up from motion detection
void clearBMI160Interrupts();

//checks if bmi160 is connected and responding
//returns true if chip id matches expected value
bool isBMI160Connected();

//reads current accelerometer values for debugging
//parameters are passed by reference to return x,y,z values
void readBMI160Accelerometer(int16_t &ax, int16_t &ay, int16_t &az);

#endif //BMI160_ACCE_H