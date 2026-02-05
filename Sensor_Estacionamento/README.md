# Arduino Ultrasonic Parking Sensor

This project simulates a car reverse parking sensor using an Arduino, an ultrasonic sensor (HC-SR04), and a 6-LED bar graph to visually indicate distance.

* Circuit Photo:** A photo/diagram of the circuit assembly is available on this page.

## Components
* **1x Arduino Board** (Uno, Nano, etc.)
* **1x HC-SR04** Ultrasonic Sensor
* **6x LEDs** (2 Green, 2 Yellow, 2 Red)
* **6x Resistors** (220Ω or 330Ω)
* **Jumper Wires & Breadboard**

## How it Works
The system measures distance in real-time. As an object approaches, LEDs light up progressively based on the distance:

* **< 60cm:** 1st Green LED ON
* **< 50cm:** 2nd Green LED ON
* **< 40cm:** 1st Yellow LED ON
* **< 30cm:** 2nd Yellow LED ON
* **< 20cm:** 1st Red LED ON
* **< 10cm:** 2nd Red LED ON
* **< 5cm (CRITICAL):** Red LEDs blink rapidly to signal collision danger.

## Pin Configuration
* **Trig Pin:** 12
* **Echo Pin:** 11
* **LEDs:** Digital Pins 2 through 7
