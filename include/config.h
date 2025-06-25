//configuration header file

//pins
#define SDA_PIN 5
#define SCL_PIN 6
#define BUTTON_PIN 4
#define ACCE_INT_PIN 2
#define DISPLAY_POWER_PIN 3

//button timing
#define DEBOUNCE_DELAY_MS 100
#define LONG_PRESS_MS_ENTER_MENU 500
#define LONG_PRESS_MENU_SELECTION 300

//menu state tracking
#define MENU_MAIN 0
#define MENU_HISTORY 1
#define MENU_DICE 2
#define MENU_CONFIG 3

//menu options count and current selection
#define MAIN_MENU_ITEMS 4
#define DICE_MENU_ITEMS 8
#define CONFIG_MENU_ITEMS 5

//default configuration values
#define BRIGHTNESS_DEFAULT 128
#define TIME_TO_CLEAR_DISPLAY_DEFAULT 3500
#define STAGGER_DEFAULT 2
#define ACCELEROMETER_DEFAULT 1
#define COIN_COUNT_DEFAULT 0
#define D4_COUNT_DEFAULT 0
#define D6_COUNT_DEFAULT 1
#define D8_COUNT_DEFAULT 0
#define D10_COUNT_DEFAULT 0
#define D12_COUNT_DEFAULT 0
#define D20_COUNT_DEFAULT 0