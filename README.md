# AMIT-EXPRESSO: Programmable USB MIDI Expression Pedal Converter

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**Project Description:**

AMIT-EXPRESSO is an Arduino-based project that transforms a standard expression pedal into a programmable USB MIDI controller. It also incorporates a sustain/damper pedal input, making it a versatile tool for musicians and producers. The project is designed to be used with an Arduino Micro Pro (Leonardo) board.

I directly converted a normal Nektar NX-P Expression Pedal into Usb Pedal by removing its original cable in putting the arduino board with a micro usb cable attached to it inside the pedal itself. However you can use this with any pedal by using a stereo input jack and wiring its three pins (tip , ring and sleeve) as per the wiring diagram image. and mounting that a lgon with arduino controller into some box. (use a plastic or insulated box for sustain jack (mono audio jack) so it does not short with ground of expession jack).


**Key Features:**

*   **Expression Pedal Input:** Converts analog expression pedal movements into MIDI Control Change (CC) messages.
*   **Sustain/Damper Pedal Input:** Accepts a standard sustain pedal (switch) and sends MIDI CC messages accordingly.
*   **Programmable CC Assignments:** Allows users to assign different MIDI CC numbers to both the expression and sustain pedals via incoming MIDI messages.
*   **Dead Zone Adjustment:** Includes a dead zone feature to compensate for low-precision potentiometers, ensuring accurate control.
*   **EEPROM Storage:** Saves the user's custom CC assignments to EEPROM, allowing them to persist across power cycles.
*   **MIDI Input Handling:** Receives MIDI messages to configure the pedal's behavior.
*   **USB MIDI Output:** Sends MIDI messages over USB, making it compatible with most DAWs and MIDI-enabled software.
* **Customizable Board ID:** Allows for custom board ID and identifiers.

**Software Components:**

1.  **Arduino IDE:** The development environment for programming the Arduino.
2.  **Libraries:**
    *   **MIDI Library:** For sending and receiving MIDI messages.
    *   **USB-MIDI Library:** For sending MIDI messages over USB.
    *   **EEPROM Library:** For storing data in the Arduino's EEPROM.
    * **ATPOTS.h/ATPOTS.cpp:** Custom library for handling potentiometers.

**Code Structure:**

*   **`AMIT-EXPRESSO.ino` (Main Sketch):**
    *   Includes necessary libraries.
    *   Defines pin configurations, MIDI channel, and other constants.
    *   Defines the `PEDALSTATE` struct for storing pedal settings.
    *   `handleMidiInput()`: Processes incoming MIDI messages to configure the pedal.
    *   `initPedal()`: Resets the pedal to default settings.
    *   `setup()`: Initializes pins, serial communication, and MIDI.
    *   `saveConfig()`: Saves the current pedal configuration to EEPROM.
    *   `loadConfig()`: Loads the pedal configuration from EEPROM.
    *   `handleSustain()`: Reads the sustain pedal state and sends MIDI CC messages.
    *   `loop()`: Continuously scans the expression pedal, handles the sustain pedal, and processes MIDI input.
*   **`ATPOTS.h` (Header File):**
    *   Defines the `ATPOT` class for handling potentiometers.
    *   Defines the `ATMIDICCPOT` class, which inherits from `ATPOT` and adds MIDI CC functionality (Serial midi only).
*   **`ATPOTS.cpp` (Implementation File):**
    *   Implements the methods of the `ATPOT` and `ATMIDICCPOT` classes.
    *   Includes functions for reading analog values, applying dead zones, mapping values, and sending MIDI CC messages.

**Class Descriptions:**

*   **`ATPOT`:**
    *   **Purpose:** Represents a generic potentiometer.
    *   **Members:**
        *   `_minVal`, `_maxVal`: Minimum and maximum output values.
        *   `_pin`: The analog pin connected to the potentiometer.
        *   `_lastReading`: The last read value.
        *   `_deadZonePercent`: The percentage of the dead zone.
        *   `_deadZoneFactor`: The calculated dead zone range.
        * `rawValue`: The raw value from the analog read.
        * `value`: The mapped value.
        * `hasChanged`: Flag to indicate if the value has changed.
    *   **Methods:**
        *   `ATPOT(byte pin, int minVal, int maxVal, float deadZonePercent)`: Constructor with custom min/max and dead zone.
        *   `ATPOT(byte pin, int minVal, int maxVal, float deadZonePercent,void (*handler)(byte, byte))`: Constructor with custom min/max and dead zone and a callBack function to call on value change.
        *   `ATPOT(byte pin, float deadZonePercent)`: Constructor with a dead zone.
        *   `ATPOT(byte pin)`: Basic constructor.
        *   `scan()`: Reads the potentiometer, applies the dead zone, maps the value, and triggers `changed()` if the value has changed.
        *   `reset()`: Resets the `hasChanged` flag.
        *   `changed()`: Virtual method called when the value changes.
        *   `aRead()`: Reads the analog pin and averages multiple readings.
*   **`ATMIDICCPOT`:**
    *   **Purpose:** Represents a potentiometer that sends MIDI CC messages.
    *   **Inheritance:** Inherits from `ATPOT`.
    *   **Members:**
        *   `_mesg`: The MIDI message type (Control Change).
        *   `_cc`: The MIDI CC number.
        *   `_varr`: An array of values to use for the pot.
        *   `_count`: The size of the `_varr` array.
        * `valueType`: Flag to indicate if the pot uses a value array.
    *   **Methods:**
        *   `ATMIDICCPOT(byte pin, byte ch, byte cc)`: Constructor.
        *   `INIT(byte ch, byte cc)`: Initializes the MIDI channel and CC number.
        *   `INIT(byte ch, byte cc, byte *values, byte count)`: Initializes the MIDI channel, CC number, and a custom value array.
        *   `changed()`: Overrides the `ATPOT::changed()` method to send MIDI CC messages.

**MIDI Control Change (CC) Implementation:**

*   **CC 33 :** Sets the MIDI CC number (0-110) for the expression pedal. A value of 0 disables the expression pedal.
*   **CC 34 :** Sets the MIDI CC number (0-110) for the sustain pedal. A value of 0 disables the sustain pedal.
*   **CC 35 :** Resets the pedal to default settings if an even value is received.
*   **CC 36 :** Saves the current configuration to EEPROM if a value of 127 is received.
*   **CC 37 ::** Loads the saved configuration from EEPROM if a value of 127 is received.
*   **CC 38 :**  (values 1-50) Sets DeadZone of Expression Pedal.
*   **CC 39 :** Sets Midi Output Channel for Expression Pedal.
*   **CC 40:** Sets Midi Output Channel for Sustain Pedal.

**Operational Flow:**

1.  **Initialization:**
    *   The `setup()` function initializes the pins, serial communication, MIDI, and loads the configuration from EEPROM.
2.  **Expression Pedal Handling:**
    *   The `POT.scan()` function in the `loop()` continuously reads the expression pedal's analog value.
    *   If the value changes, the `changed()` method in `ATMIDICCPOT` is triggered, sending a MIDI CC message with the assigned CC number and the mapped value.
3.  **Sustain Pedal Handling:**
    *   The `handleSustain()` function in the `loop()` reads the state of the sustain pedal.
    *   If the state changes, a MIDI CC message is sent with the assigned CC number and a value of 127 (pressed) or 0 (released).
4.  **MIDI Input Handling:**
    *   The `handleMidiInput()` function in the `loop()` processes incoming MIDI messages.
    *   If a Control Change message is received with the `setEXP`, `setSustain`, `pedalReset`, `pedalSave`, or `pedalLoad` CC numbers, the corresponding action is performed.
5. **EEPROM Handling:**
    * `saveConfig()` saves the current ECC, SCC and ID to the EEPROM.
    * `loadConfig()` loads the saved config from the EEPROM. If no config is found, it sets the default values and saves them.

**How to Use:**

1.  **Upload Code:** Upload the `AMIT-EXPRESSO.ino` sketch to the Arduino Micro Pro using the Arduino IDE.
2.  **Configure CCs:**
    *   Send MIDI CC messages to the Arduino with the following CC numbers:
        *   CC 33 (setEXP): Set the CC number for the expression pedal.
        *   CC 34 (setSustain): Set the CC number for the sustain pedal.
        *   CC 35 (pedalReset): Send an even value to reset to defaults.
        *   CC 36 (pedalSave): Send 127 to save the current config.
        *   CC 37 (pedalLoad): Send 127 to load the saved config.
3.  **Use with DAW:** Connect the Arduino to your computer via USB. It will appear as a MIDI device. Configure your DAW to receive MIDI input from the Arduino.

**Potential Improvements:**

*   **Multiple Expression Pedals:** Add support for more than one expression pedal.
*   **More Programmable Switches:** Add more switch inputs for additional control.
*   **LED Feedback:** Use LEDs to provide visual feedback on the pedal's state.
*   **Encoder Support:** Add support for rotary encoders for more complex control.
* **Web Interface:** Create a web interface for configuration.

**License:**

*   Personal Use: Free to use for personal, non-commercial purposes.
*   Commercial Use: Prohibited from building and selling this project for profit.

**Author:**

*   Amit Talwar (www.amitszone.com)

---

**NOTES**
*   Gemini AI was used to generate this documentation and Code Comments.
*   If you want this arduino to show as Amits Expresso Midi controller or your desireable product name, copy the incluced hardware folder to your Documents/Arduino folder. If the Folder alredy exiss, copy the contents of included hardware folder to your Documents/Arduino/hardware folder. You can Ediit the boards.txt file to change the Name.