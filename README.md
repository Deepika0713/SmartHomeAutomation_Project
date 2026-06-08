# Smart Home Automation using Hand Gestures 🏠🖐️

An interactive, computer-vision-based Smart Home Automation system that allows users to control household appliances (simulated using 4 LEDs) via real-time hand gestures. The project leverages **OpenCV** and **MediaPipe** for hand tracking and gesture recognition on a host computer, which communicates state changes to an **Arduino Uno** via serial communication.

## 🚀 Features
* **Real-time Hand Tracking:** Uses Google MediaPipe for high-accuracy, low-latency landmark detection.
* **Intelligent Gesture Mapping:** Translates specific finger counts or configurations into system commands.
* **Multi-Appliance Control:** Independently toggles 4 different LEDs (representing rooms, lights, or appliances).
* **Robust Hardware Interface:** Seamless asynchronous serial communication between Python and Arduino.

## 🛠️ System Architecture
1. **Input Stage:** The webcam captures the live video stream.
2. **Processing Stage:** `4_leds.py` processes each frame, detects the hand, counts extended fingers, and determines the target state.
3. **Communication Stage:** Python sends a specific character encoding over the COM port (`pySerial`).
4. **Execution Stage:** `sketch_4_leds.ino` reads the incoming serial data and switches the corresponding digital pins `HIGH` or `LOW`.

## 📂 Repository Structure

├── 4_leds.py            # Python script for hand tracking, gesture logic, and serial output
├── sketch_4_leds.ino    # Arduino sketch for receiving commands and controlling GPIO pins
├── requirements.txt     # Python dependencies
└── README.md            # Project documentation

## 🔌 Hardware Setup
To build this project, you will need:
* Arduino Uno (or compatible microcontroller)
* 4x LEDs (e.g., Red, Green, Blue, Yellow)
* 4x 220Ω Resistors
* Breadboard and Jump wire cables
* USB Cable (for Serial Interface)

### Circuit Connections
* **LED 1:** Connected to Arduino **Digital Pin 2** (via 220Ω resistor)
* **LED 2:** Connected to Arduino **Digital Pin 3** (via 220Ω resistor)
* **LED 3:** Connected to Arduino **Digital Pin 4** (via 220Ω resistor)
* **LED 4:** Connected to Arduino **Digital Pin 5** (via 220Ω resistor)
* **GND:** All LED cathodes connected to Arduino **GND**
