# LED Color Control with Arduino Nano 33 BLE Sense and R4 UNO

This project demonstrates the control of red and green colors on an RGB LED strip using the Arduino Nano 33 BLE Sense, with the current color displayed on an LCD display. The code is based on the Micro Speech example provided by the Arduino_TensorFlowLite library. For power supply reasons, the circuit also includes an Arduino R4 UNO. The Nano 33 BLE Sense only provides 3.3V output, but the LEDs require 5V, so the UNO supplies the additional power. The circuit was connected according to the images provided in this repository.

---

## Overview

The project uses the Micro Speech example model to recognize simple voice commands. When the model detects "yes", the LED strip turns green and the display shows "GREEN". When the model detects "no", the LED strip turns red and the display shows "RED". All other detections are classified as "unknown" and do not change the LEDs or display.

---

## Components Used

The components used in this project include:

- Arduino Nano 33 BLE Sense
- Arduino R4 UNO for additional power
- RGB LED strip with red and green LEDs
- Standard kit LCD display, controlled via the LiquidCrystal library
- Jumper wires and breadboard for connections

---

## How It Works

The Micro Speech example model runs on the Nano 33 BLE Sense. It continuously listens for voice input and classifies it into three categories: "yes", "no", and "unknown". According to the classification, it performs the following actions:

- "yes": LED strip turns green, display shows "GREEN"
- "no": LED strip turns red, display shows "RED"
- "unknown": no action is taken

The Arduino R4 UNO provides the additional 5V power required for the LEDs, while the Nano 33 BLE Sense runs the model and updates the display. The connections of the circuit match the images provided in this repository.

---

## How to Run

Place the `.ino` file from this folder and replace the code in the example file in the Arduino IDE with this file. Upload it to the Nano 33 BLE Sense. Once running, the LEDs and display will react according to the model's predictions.

---

## Notes

- The circuit includes both the Nano 33 BLE Sense and the R4 UNO for power supply reasons.
- The LCD display is a standard kit display with blue background and white letters, controlled via the LiquidCrystal library.
- Images of the project are included inside this folder.
