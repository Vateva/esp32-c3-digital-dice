#include "lsm6ds3_acce.h"
#include "menu.h"
#include <Wire.h>

// lsm6ds3 i2c configuration
const int LSM6DS3_I2C_ADDRESS =
    0x6B; // lsm6ds3 address when sa0 = vdd (use 0x6a when sa0 = gnd)

// lsm6ds3 register addresses
#define LSM6DS3_FUNC_CFG_ACCESS 0x01
#define LSM6DS3_INT1_CTRL 0x0D
#define LSM6DS3_WHO_AM_I 0x0F
#define LSM6DS3_CTRL1_XL 0x10
#define LSM6DS3_CTRL2_G 0x11
#define LSM6DS3_CTRL3_C 0x12
#define LSM6DS3_CTRL4_C 0x13
#define LSM6DS3_CTRL5_C 0x14
#define LSM6DS3_CTRL6_C 0x15
#define LSM6DS3_CTRL7_G 0x16
#define LSM6DS3_CTRL8_XL 0x17
#define LSM6DS3_CTRL9_XL 0x18
#define LSM6DS3_CTRL10_C 0x19
#define LSM6DS3_WAKE_UP_SRC 0x1B
#define LSM6DS3_TAP_CFG 0x58
#define LSM6DS3_WAKE_UP_THS 0x5B
#define LSM6DS3_WAKE_UP_DUR 0x5C
#define LSM6DS3_FREE_FALL 0x5D
#define LSM6DS3_MD1_CFG 0x5E
#define LSM6DS3_OUTX_L_XL 0x28
#define LSM6DS3_OUTX_H_XL 0x29
#define LSM6DS3_OUTY_L_XL 0x2A
#define LSM6DS3_OUTY_H_XL 0x2B
#define LSM6DS3_OUTZ_L_XL 0x2C
#define LSM6DS3_OUTZ_H_XL 0x2D

// expected chip id for lsm6ds3
#define LSM6DS3_CHIP_ID 0x69

// internal function prototypes
static uint8_t readRegister(uint8_t reg);
static void writeRegister(uint8_t reg, uint8_t value);
static void configureLSM6DS3WakeUpDetection();

// reads a single register from lsm6ds3
static uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(LSM6DS3_I2C_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(LSM6DS3_I2C_ADDRESS, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}

// writes a value to a single register in lsm6ds3
static void writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(LSM6DS3_I2C_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

// initializes lsm6ds3 and configures it for wake-up detection
bool initLSM6DS3() {
  // configure motion interrupt pin as input with pullup
  pinMode(MOTION_INT_PIN, INPUT_PULLUP);

  // check if lsm6ds3 is connected
  if (!isLSM6DS3Connected()) {
    return false;
  }

  // software reset
  writeRegister(LSM6DS3_CTRL3_C, 0x01);
  delay(50); // wait for reset to complete

  // enable block data update (bdu) to ensure consistent data reads
  //enable bit 0x20 to make int LOW
  writeRegister(LSM6DS3_CTRL3_C, 0x64);
  delay(10);

  // set accelerometer to 52hz for low-power mode, ±2g
  // odr=52hz allows low-power mode when xl_hm_mode=1
  writeRegister(LSM6DS3_CTRL1_XL, 0x30);
  delay(10);

  // enable low-power mode for accelerometer
  // xl_hm_mode=1 + 52hz odr = low-power mode
  writeRegister(LSM6DS3_CTRL6_C, 0x10);
  delay(10);

  // keep gyroscope in power down mode to save power
  writeRegister(LSM6DS3_CTRL2_G, 0x00);
  delay(10);

  // enable embedded functions only (no need to enable gyro axes)
  // func_en=1, all other bits=0
  writeRegister(LSM6DS3_CTRL10_C, 0x04); // Only FUNC_EN=1
  delay(10);

  // configure wake-up detection parameters
  configureLSM6DS3WakeUpDetection();

  return true;
}

// checks if lsm6ds3 is connected and responding correctly
bool isLSM6DS3Connected() {
  uint8_t chipId = readRegister(LSM6DS3_WHO_AM_I);
  return (chipId == LSM6DS3_CHIP_ID);
}

// configures all wake-up detection parameters
static void configureLSM6DS3WakeUpDetection() {
  // lir=1 to latch interrupts until cleared, 0 for pulse
  writeRegister(LSM6DS3_TAP_CFG, 0x00);
  delay(10);

  // set wake-up threshold
  writeRegister(LSM6DS3_WAKE_UP_THS, 0x1B);
  delay(10);

  // set wake-up duration
  // wake_dur[1:0] = 00 (no additional duration required)
  // sleep_dur[3:0] = 0000 (sleep mode disabled)
  writeRegister(LSM6DS3_WAKE_UP_DUR, 0x40);
  delay(10);

  // route wake-up interrupt to int1 pin
  // int1_wu = 1 (bit 5)
  writeRegister(LSM6DS3_MD1_CFG, 0x20);
  delay(10);

  // additional int1 configuration can be done here if needed
  // for now, wake-up routing through md1_cfg is sufficient

  // clear any existing interrupts
  clearLSM6DS3Interrupts();

  // read a few samples to stabilize
  int16_t ax, ay, az;
  for (int i = 0; i < 3; i++) {
    readLSM6DS3Accelerometer(ax, ay, az);
    delay(50);
  }

  // final interrupt clear
  clearLSM6DS3Interrupts();
}

// prepares lsm6ds3 for deep sleep mode
void configureLSM6DS3ForSleep() {
  // wake-up detection is already configured during init
  // this function is here for any future sleep-specific settings

  // ensure accelerometer is still in normal mode for wake-up detection
  // lsm6ds3 wake-up function works in normal mode

  // clear any pending interrupts before sleep
  clearLSM6DS3Interrupts();
}

void disableLSM6DS3() {
  // power down accelerometer (CTRL1_XL register)
  writeRegister(LSM6DS3_CTRL1_XL, 0x00);  // ODR_XL[3:0] = 0000 = Power-down
  delay(10);
  
  // disable embedded functions (CTRL10_C register) 
  writeRegister(LSM6DS3_CTRL10_C, 0x00);  // FUNC_EN = 0 disables wake-up detection
  delay(10);
  
  // disable accelerometer axes (CTRL9_XL register)
  writeRegister(LSM6DS3_CTRL9_XL, 0x00);  // Disable X,Y,Z axes outputs
  delay(10);
  
  // disable wake-up interrupt routing (MD1_CFG register)
  writeRegister(LSM6DS3_MD1_CFG, 0x00);   // Disable all interrupt routing
  delay(10);
  
  // clear any pending interrupts
  readRegister(LSM6DS3_WAKE_UP_SRC);
  delay(10);
}

// clears any pending interrupts from lsm6ds3
void clearLSM6DS3Interrupts() {
  // read wake-up source register to clear wake-up interrupt
  readRegister(LSM6DS3_WAKE_UP_SRC);

  // small delay to ensure clearing is complete
  delay(5);
}

// reads current accelerometer values
void readLSM6DS3Accelerometer(int16_t &ax, int16_t &ay, int16_t &az) {
  // read 6 bytes starting from outx_l_xl register
  Wire.beginTransmission(LSM6DS3_I2C_ADDRESS);
  Wire.write(LSM6DS3_OUTX_L_XL);
  Wire.endTransmission(false);
  Wire.requestFrom(LSM6DS3_I2C_ADDRESS, 6);

  if (Wire.available() >= 6) {
    // read x-axis (little endian)
    uint8_t xl = Wire.read();
    uint8_t xh = Wire.read();
    ax = (int16_t)((xh << 8) | xl);

    // read y-axis (little endian)
    uint8_t yl = Wire.read();
    uint8_t yh = Wire.read();
    ay = (int16_t)((yh << 8) | yl);

    // read z-axis (little endian)
    uint8_t zl = Wire.read();
    uint8_t zh = Wire.read();
    az = (int16_t)((zh << 8) | zl);
  } else {
    // if not enough data available, set to zero
    ax = ay = az = 0;
  }
}