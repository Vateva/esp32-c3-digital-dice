# esp32-c3-digital-dice
Simple digital dice using ESP32-C3 Super Mini with 1.3" OLED display and momentary button.
Main objective is to learn more about git/github and microcontrollers/c.

## Hardware Components
- ESP32 C3 Super Mini
- 1.3" Display(SH1106)
- Momentary Push Button
- Dupont wires


## Test Wiring
The idea is to print a cubical casing and power the whole set upt with a button cell, probably CR2477 as we will probably have vertical space to spare, but first we will build a working prototype powered by the usb-c port.
- [x] Soldered pins to ESP32 C3 Super Mini
![esp32c3superminipins](https://github.com/user-attachments/assets/8eced715-a916-418f-835d-47435327fa81)
- [x] Common ground wire
      ![commongroundupont](https://github.com/user-attachments/assets/dc3a87b4-1a35-4714-a49f-55c347cf56cc)

- [x] Button wiring

![button wiring](https://github.com/user-attachments/assets/99b9bcc6-91e5-4356-a87c-d4289aa7ec71)
- [x] Prototype for testing
![nobatterynocasing test wiring](https://github.com/user-attachments/assets/b091df99-b107-424f-84ea-bf5e541a1e10)

Now we use testhardware.cpp to test the setup.
![testhardware](https://github.com/user-attachments/assets/959974c1-7d6c-4c69-ba8f-38003629b8c3)

Everything works!
