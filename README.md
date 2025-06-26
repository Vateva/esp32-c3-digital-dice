# ESP32-C3 Digital Dice

Simple digital dice using ESP32-C3 Super Mini with 1.3" OLED display and momentary button.

Main objective is to learn more about git/github and microcontrollers/C.

## Hardware Components

- ESP32 C3 Super Mini
- 1.3" Display (SH1106)
- Momentary Push Button
- LSM6DS3 Accelerometer
- Small 500mAh pouch LiPo
- TP4057 1A Charging module
- Small switch
- Dupont wires

## Test Wiring

The idea was to print a cubical casing and power the whole setup with a button cell, probably CR2477 as we will probably have vertical space to spare, but first we will build a working prototype powered by the USB-C port.

### Soldered pins to ESP32 C3 Super Mini

<div align="center">
    <img src="https://github.com/user-attachments/assets/8eced715-a916-418f-835d-47435327fa81" width="450" />
</div>

### Common ground wire

<div align="center">
    <img src="https://github.com/user-attachments/assets/dc3a87b4-1a35-4714-a49f-55c347cf56cc" width="450" />
</div>

### Button wiring

<div align="center">
    <img src="https://github.com/user-attachments/assets/99b9bcc6-91e5-4356-a87c-d4289aa7ec71" width="450" />
</div>

### Prototype for testing

<div align="center">
    <img src="https://github.com/user-attachments/assets/b091df99-b107-424f-84ea-bf5e541a1e10" width="450" />
</div>

Now we use `testhardware.cpp` to test the setup.

<div align="center">
    <img src="https://github.com/user-attachments/assets/959974c1-7d6c-4c69-ba8f-38003629b8c3" width="450" />
</div>

**Everything works!**

## Final Product

I ended up adding a few QoL upgrades to the initial idea/prototype:

- A small switch to turn power off completely between sessions
- A rechargeable 500mAh LiPo pouch, rechargeable through USB-C with a small TP4057 1A
- A LSM6DS3 accelerometer to add shake-to-roll capabilities

For the casing I decided to learn very basic Blender and designed and printed it myself with an Ender 3 Pro I got from a friend. The STL file will be in the repository in case someone wants to replicate the whole project.

### Wired Components (before casing)

Final product looks like this with everything wired but not placed inside the casing:

<div align="center">
    <img src="https://github.com/user-attachments/assets/5a5fe65d-ffd2-43f7-b5d6-8ed447fa7e7d" width="450" />
</div>

<div align="center">
    <img src="https://github.com/user-attachments/assets/62b013cf-bb54-45e4-b1a7-99bc85e18d8d" width="450" />
</div>


I glued the TP4057 1A on top of the pouch and glued it to the bottom of the casing. I also glued the ESP32-C3 to the side of the casing and added a small hydrogel film on top of the screen before placing it in the casing for extra protection.

### Final Product (in casing)

<div align="center">
    <img src="https://github.com/user-attachments/assets/80930081-2158-40fb-9822-0a78e254b7d7" width="450" />
</div>

<div align="center">
    <img src="https://github.com/user-attachments/assets/f2e6b35b-14b9-4382-af63-a8d1ebb66420" width="450" />
</div>

<div align="center">
    <img src="https://github.com/user-attachments/assets/f372c949-f1df-43bc-8603-d781ff4e1637" width="450" />
</div>

## Demonstration Videos

Here are a couple of videos showcasing the dice rolling and the menu:

# ESP32-C3 Digital Dice

Simple digital dice using ESP32-C3 Super Mini with 1.3" OLED display and momentary button.

Main objective is to learn more about git/github and microcontrollers/C.

## Hardware Components

- ESP32 C3 Super Mini
- 1.3" Display (SH1106)
- Momentary Push Button
- LSM6DS3 Accelerometer
- Small 500mAh pouch LiPo
- TP4057 1A Charging module
- Small switch
- Dupont wires

## Test Wiring

The idea was to print a cubical casing and power the whole setup with a button cell, probably CR2477 as we will probably have vertical space to spare, but first we will build a working prototype powered by the USB-C port.

### Soldered pins to ESP32 C3 Super Mini

<div align="center">
    <img src="https://github.com/user-attachments/assets/8eced715-a916-418f-835d-47435327fa81" width="450" />
</div>

### Common ground wire

<div align="center">
    <img src="https://github.com/user-attachments/assets/dc3a87b4-1a35-4714-a49f-55c347cf56cc" width="450" />
</div>

### Button wiring

<div align="center">
    <img src="https://github.com/user-attachments/assets/99b9bcc6-91e5-4356-a87c-d4289aa7ec71" width="450" />
</div>

### Prototype for testing

<div align="center">
    <img src="https://github.com/user-attachments/assets/b091df99-b107-424f-84ea-bf5e541a1e10" width="450" />
</div>

Now we use `testhardware.cpp` to test the setup.

<div align="center">
    <img src="https://github.com/user-attachments/assets/959974c1-7d6c-4c69-ba8f-38003629b8c3" width="450" />
</div>

**Everything works!**

## Final Product

I ended up adding a few QoL upgrades to the initial idea/prototype:

- A small switch to turn power off completely between sessions
- A rechargeable 500mAh LiPo pouch, rechargeable through USB-C with a small TP4057 1A
- A LSM6DS3 accelerometer to add shake-to-roll capabilities

For the casing I decided to learn very basic Blender and designed and printed it myself with an Ender 3 Pro I got from a friend. The STL file will be in the repository in case someone wants to replicate the whole project.

### Wired Components (before casing)

Final product looks like this with everything wired but not placed inside the casing:

<div align="center">
    <img src="https://github.com/user-attachments/assets/5a5fe65d-ffd2-43f7-b5d6-8ed447fa7e7d" width="450" />
</div>

<div align="center">
    <img src="https://github.com/user-attachments/assets/62b013cf-bb54-45e4-b1a7-99bc85e18d8d" width="450" />
</div>

### Assembly Process

I glued the TP4057 1A on top of the pouch and glued it to the bottom of the casing. I also glued the ESP32-C3 to the side of the casing and added a small hydrogel film on top of the screen before placing it in the casing for extra protection.

### Final Product (in casing)

<div align="center">
    <img src="https://github.com/user-attachments/assets/80930081-2158-40fb-9822-0a78e254b7d7" width="450" />
</div>

<div align="center">
    <img src="https://github.com/user-attachments/assets/f2e6b35b-14b9-4382-af63-a8d1ebb66420" width="450" />
</div>

<div align="center">
    <img src="https://github.com/user-attachments/assets/f372c949-f1df-43bc-8603-d781ff4e1637" width="450" />
</div>

## Videos

Here are a couple of videos showcasing the dice rolling and the menu:
# ESP32-C3 Digital Dice

Simple digital dice using ESP32-C3 Super Mini with 1.3" OLED display and momentary button.

Main objective is to learn more about git/github and microcontrollers/C.

## Hardware Components

- ESP32 C3 Super Mini
- 1.3" Display (SH1106)
- Momentary Push Button
- LSM6DS3 Accelerometer
- Small 500mAh pouch LiPo
- TP4057 1A Charging module
- Small switch
- Dupont wires

## Test Wiring

The idea was to print a cubical casing and power the whole setup with a button cell, probably CR2477 as we will probably have vertical space to spare, but first we will build a working prototype powered by the USB-C port.

### Soldered pins to ESP32 C3 Super Mini

<div align="center">
    <img src="https://github.com/user-attachments/assets/8eced715-a916-418f-835d-47435327fa81" width="450" />
</div>

### Common ground wire

<div align="center">
    <img src="https://github.com/user-attachments/assets/dc3a87b4-1a35-4714-a49f-55c347cf56cc" width="450" />
</div>

### Button wiring

<div align="center">
    <img src="https://github.com/user-attachments/assets/99b9bcc6-91e5-4356-a87c-d4289aa7ec71" width="450" />
</div>

### Prototype for testing

<div align="center">
    <img src="https://github.com/user-attachments/assets/b091df99-b107-424f-84ea-bf5e541a1e10" width="450" />
</div>

Now we use `testhardware.cpp` to test the setup.

<div align="center">
    <img src="https://github.com/user-attachments/assets/959974c1-7d6c-4c69-ba8f-38003629b8c3" width="450" />
</div>

**Everything works!**

## Final Product

I ended up adding a few QoL upgrades to the initial idea/prototype:

- A small switch to turn power off completely between sessions
- A rechargeable 500mAh LiPo pouch, rechargeable through USB-C with a small TP4057 1A
- A LSM6DS3 accelerometer to add shake-to-roll capabilities

For the casing I decided to learn very basic Blender and designed and printed it myself with an Ender 3 Pro I got from a friend. The STL file will be in the repository in case someone wants to replicate the whole project.

### Wired Components (before casing)

Final product looks like this with everything wired but not placed inside the casing:

<div align="center">
    <img src="https://github.com/user-attachments/assets/5a5fe65d-ffd2-43f7-b5d6-8ed447fa7e7d" width="450" />
</div>

<div align="center">
    <img src="https://github.com/user-attachments/assets/62b013cf-bb54-45e4-b1a7-99bc85e18d8d" width="450" />
</div>

### Assembly Process

I glued the TP4057 1A on top of the pouch and glued it to the bottom of the casing. I also glued the ESP32-C3 to the side of the casing and added a small hydrogel film on top of the screen before placing it in the casing for extra protection.

### Final Product (in casing)

<div align="center">
    <img src="https://github.com/user-attachments/assets/80930081-2158-40fb-9822-0a78e254b7d7" width="450" />
</div>

<div align="center">
    <img src="https://github.com/user-attachments/assets/f2e6b35b-14b9-4382-af63-a8d1ebb66420" width="450" />
</div>

<div align="center">
    <img src="https://github.com/user-attachments/assets/f372c949-f1df-43bc-8603-d781ff4e1637" width="450" />
</div>

## Demonstration Videos

Here are a couple of videos showcasing the dice rolling and the menu:

### Dice Roll 

<div align="center">
    <video width="450" controls>
        <source src="https://github.com/user-attachments/assets/2823d9ca-534b-4626-b369-a73eaf9733b9" type="video/mp4">
    </video>
</div>

### Menu

<div align="center">
    <video width="450" controls>
        <source src="https://github.com/user-attachments/assets/94b1bece-dcf2-499f-b2bd-8fa810c06be8" type="video/mp4">
    </video>
</div>
