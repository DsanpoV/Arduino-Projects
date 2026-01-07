# Smart Fan Control System Using TinyML and Multi-Arduino Architecture

## Abstract

This project presents an intelligent fan control system based on environmental sensing, user behavior history, and embedded machine learning (TinyML). The system integrates multiple Arduino boards with a PC-based Python processing layer, enabling data acquisition, model training, and real-time decision inference directly on constrained hardware.

---

## System Overview

The system is designed around a modular architecture composed of three layers:

1. **Sensor and Actuation Layer (Arduino Hardware)**
2. **Processing and Communication Layer (PC – Python)**
3. **Embedded Intelligence Layer (TinyML on Arduino Nano 33 BLE)**

The objective is to autonomously control a ventilation system based on meteorological variables and learned user preferences.

---

## Repository Structure

---

## 1. Hardware Layer (Arduino Code)

### `arduino_r4_main/`
Code for the **Arduino R4 WiFi**.

**Functionality:**
- Reads temperature and humidity sensors
- Reads RFID data
- Sends all sensor data to the PC via serial communication

This board is dedicated exclusively to data acquisition.

---

### `SmartFan.ino/`
Code for the **Arduino Nano 33 BLE**, responsible for decision-making and actuation.

**Functionality:**
- Receives processed sensor data from the PC
- Runs a TinyML neural network model locally
- Determines whether to activate or deactivate the fan
- Controls the fan output accordingly

All inference is performed directly on the microcontroller.

---

### `nano33_ble_voice/`
Code for the **Arduino Nano 33 BLE** used during dataset generation.

**Functionality:**
- Voice recognition of the commands “Yes” and “No”
- Manual fan control during data collection
- User preference labeling for supervised learning

This module is used only during the data acquisition phase.

---

## 2. PC Layer (Python Processing)

### `SmartPonte.py`
**Main execution script of the system.**

**Responsibilities:**
- Establish serial communication with:
  - Arduino R4 WiFi
  - Arduino Nano 33 BLE
- Read raw sensor data from the Arduino R4
- Process and normalize the data
- Transmit the processed data to the Nano 33 BLE for inference

This script acts as the communication and processing bridge between hardware components.

---

### `Model_sense33.py`
**Machine learning training script.**

**Responsibilities:**
- Load historical data from the CSV dataset
- Train a neural network model
- Export the trained model as a C header file (`fan_model.h`)
- Generate a model compatible with TinyML constraints

The generated model must be manually copied into the `SmartFan.ino` project before compilation.

---

### `ArduinoCsv.py`
**Initial data collection utility.**

**Responsibilities:**
- Read sensor data during the training phase
- Store sensor values and user decisions in a CSV file
- Generate the dataset used for model training

This script is not required during normal system operation.

---

## 3. Dataset

### `dataset_casa_inteligente2.csv`

The dataset contains:
- Environmental sensor readings
- User decisions (fan ON / OFF)
- Contextual and temporal information

This dataset is used for supervised training of the TinyML model.

---

## Operational Workflow

1. **Data Collection**
   - Sensor readings and voice commands
   - Data stored using `ArduinoCsv.py`

2. **Model Training**
   - Execute `Model_sense33.py`
   - Generate `fan_model.h`

3. **Deployment**
   - Insert the trained model into `SmartFan.ino`
   - Flash the Arduino Nano 33 BLE

4. **System Execution**
   - Run `SmartPonte.py`
   - System operates autonomously

---

## Technical Notes

- The PC is required only for data processing and inter-device communication.
- All decision inference is performed locally on the Arduino Nano 33 BLE.
- The architecture supports future extensions, including additional sensors or alternative models.
