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


## 💻 Software Installation & Configuration

### Prerequisites
Ensure you have Python 3.8+ and the Arduino IDE installed.

### 1. Arduino Setup
1. Open the Arduino IDE.
2. Open the file `sketch_4_leds.ino`.
3. Connect your Arduino board to your computer.
4. Select the correct **Board** and **Port** from the *Tools* menu.
5. Click **Upload**.
6. *Note the COM Port number (e.g., COM3 on Windows or /dev/ttyACM0 on Linux/macOS).*

### 2. Python Setup
1. Clone this repository to your local machine:
   ```bash
   git clone [https://github.com/Deepika0713/SmartHomeAutomation_Project.git](https://github.com/Deepika0713/SmartHomeAutomation_Project.git)
   cd SmartHomeAutomation_Project
Install the required libraries:

Bash
pip install -r requirements.txt
Open 4_leds.py and modify the serial port initialization line to match your Arduino's port:

Python
# Example for Windows:
arduino = serial.Serial('COM3', 9600, timeout=1) 

# Example for Linux/Mac:
 arduino = serial.Serial('/dev/ttyACM0', 9600, timeout=1)

How to Run
Ensure your Arduino is plugged into the USB port.

Run the Python application:

Bash
python 4_leds.py
A window showing your webcam feed will open.

Hold your hand up to the camera to begin controlling the LEDs:

Gesture Examples (Standard configuration):

1 Finger Extended ➡️ Toggle/Turn on Light 1

2 Fingers Extended ➡️ Toggle/Turn on Light 2

3 Fingers Extended ➡️ Toggle/Turn on Light 3

4 Fingers Extended ➡️ Toggle/Turn on Light 4

Open Palm (5) or Fist (0) ➡️ Master command (All ON / All OFF depending on code setup)

Press q while focusing on the camera window to safely exit the application and close the serial stream.

✨ Future Enhancements
Integration with physical relay modules to control real 230V household appliances.

Adding voice control overlay alongside gesture recognition.

Developing a lightweight GUI dashboard to monitor live device states.


