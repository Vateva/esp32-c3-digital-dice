#include "bmi160_acce.h"
#include <Wire.h>

//bmi160 i2c configuration
const int BMI160_I2C_ADDRESS = 0x69;  //bmi160 address when sa0 = vcc

//bmi160 register addresses
#define BMI160_CHIP_ID_REG      0x00
#define BMI160_ACC_DATA_REG     0x12
#define BMI160_GYR_DATA_REG     0x0C
#define BMI160_CMD_REG          0x7E
#define BMI160_ACC_CONF         0x40
#define BMI160_ACC_RANGE        0x41
#define BMI160_INT_EN_0         0x50
#define BMI160_INT_EN_1         0x51
#define BMI160_INT_OUT_CTRL     0x53
#define BMI160_INT_MAP_0        0x55
#define BMI160_INT_MAP_1        0x56
#define BMI160_INT_MOTION_0     0x5F
#define BMI160_INT_MOTION_1     0x60
#define BMI160_INT_MOTION_2     0x61
#define BMI160_INT_MOTION_3     0x62
#define BMI160_INT_STATUS_0     0x1C
#define BMI160_INT_STATUS_1     0x1D

//bmi160 commands
#define BMI160_ACC_NORMAL_MODE  0x11
#define BMI160_GYR_NORMAL_MODE  0x15

//expected chip id for bmi160
#define BMI160_CHIP_ID          0xD3

//internal function prototypes
static uint8_t readRegister(uint8_t reg);
static void writeRegister(uint8_t reg, uint8_t value);
static void configureBMI160MotionDetection();

//reads a single register from bmi160
static uint8_t readRegister(uint8_t reg) {
    Wire.beginTransmission(BMI160_I2C_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(BMI160_I2C_ADDRESS, 1);
    return Wire.read();
}

//writes a value to a single register in bmi160
static void writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(BMI160_I2C_ADDRESS);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

//initializes bmi160 and configures it for motion detection
bool initBMI160() {
    //configure motion interrupt pin as input with pulldown
    pinMode(MOTION_INT_PIN, INPUT_PULLDOWN);
    
    //check if bmi160 is connected
    if (!isBMI160Connected()) {
        return false;
    }
    
    //initialize accelerometer to normal mode
    writeRegister(BMI160_CMD_REG, BMI160_ACC_NORMAL_MODE);
    delay(100); //wait for mode change to complete
    
    //configure motion detection parameters
    configureBMI160MotionDetection();
    
    return true;
}

//checks if bmi160 is connected and responding correctly
bool isBMI160Connected() {
    uint8_t chipId = readRegister(BMI160_CHIP_ID_REG);
    return (chipId == BMI160_CHIP_ID);
}

//configures all motion detection parameters
static void configureBMI160MotionDetection() {
    //set accelerometer range to ±2g for better motion sensitivity
    writeRegister(BMI160_ACC_RANGE, 0x03);
    delay(10);
    
    //set accelerometer bandwidth and output data rate to 100hz
    writeRegister(BMI160_ACC_CONF, 0x28);
    delay(10);
    
    //set motion detection threshold (0x40 = ~0.5g threshold)
    writeRegister(BMI160_INT_MOTION_1, 0x40);
    delay(10);
    
    //set motion detection duration (0x01 = 2 consecutive samples)
    writeRegister(BMI160_INT_MOTION_0, 0x01);
    delay(10);
    
    //enable motion detection interrupt on all axes
    writeRegister(BMI160_INT_EN_0, 0x07);
    delay(10);
    
    //configure interrupt output as active high push-pull
    writeRegister(BMI160_INT_OUT_CTRL, 0x0A);
    delay(10);
    
    //map motion interrupt to int1 pin
    writeRegister(BMI160_INT_MAP_0, 0x07);
    delay(10);
    
    //clear any existing interrupts
    clearBMI160Interrupts();
    
    //read a few samples to stabilize
    int16_t ax, ay, az;
    for (int i = 0; i < 3; i++) {
        readBMI160Accelerometer(ax, ay, az);
        delay(50);
    }
    
    //final interrupt clear
    clearBMI160Interrupts();
}

//prepares bmi160 for deep sleep mode
void configureBMI160ForSleep() {
    //motion detection is already configured during init
    //this function is here for any future sleep-specific settings
    //currently just clears interrupts one more time
    clearBMI160Interrupts();
}

//clears any pending interrupts from bmi160
void clearBMI160Interrupts() {
    //read interrupt status registers to clear them
    readRegister(BMI160_INT_STATUS_0);
    readRegister(BMI160_INT_STATUS_1);
    
    //small delay to ensure clearing is complete
    delay(5);
}

//reads current accelerometer values
void readBMI160Accelerometer(int16_t &ax, int16_t &ay, int16_t &az) {
    Wire.beginTransmission(BMI160_I2C_ADDRESS);
    Wire.write(BMI160_ACC_DATA_REG);
    Wire.endTransmission(false);
    Wire.requestFrom(BMI160_I2C_ADDRESS, 6);
    
    if (Wire.available() >= 6) {
        //read 6 bytes for x,y,z values (2 bytes each, little endian)
        ax = Wire.read() | (Wire.read() << 8);
        ay = Wire.read() | (Wire.read() << 8);
        az = Wire.read() | (Wire.read() << 8);
    }
}