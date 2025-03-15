/**
 * @file AMIT-EXPRESSO.ino
 * @brief Main Arduino sketch for the AMIT-EXPRESSO project, a programmable USB MIDI expression pedal converter.
 * @author Amit Talwar (www.amitszone.com)
 * @license Personal Use: Free to use for personal, non-commercial purposes.
 *          Commercial Use: Prohibited from building and selling this project for profit.
 * @version 1.0
 * @date 2025-03-14
 *
 * @details This sketch transforms an analog expression pedal into a programmable USB MIDI controller.
 *          It also supports a sustain/damper pedal input. The project is designed for the Arduino Micro Pro (Leonardo).
 *
 * @section Features
 *   - Converts analog expression pedal movements into MIDI Control Change (CC) messages.
 *   - Accepts a standard sustain pedal (switch) and sends MIDI CC messages accordingly.
 *   - Programmable CC assignments for both expression and sustain pedals via incoming MIDI messages.
 *   - Dead zone adjustment to compensate for low-precision potentiometers.
 *   - EEPROM storage for persistent CC assignments across power cycles.
 *   - MIDI input handling for configuration.
 *   - USB MIDI output for compatibility with DAWs and MIDI-enabled software.
 *   - Customizable board ID.
 *
 * @section Components
 *   - Arduino Micro Pro (Leonardo)
 *   - Analog expression pedal
 *   - Mono input jack for sustain pedal
 *   - 10k resistor
 *   - Connection wires
 *
 * @section Wiring
 *   - **Sustain Pedal Input:**
 *     - Solder a 10k resistor between pin 2 and Ground (GND) on the Arduino.
 *     - Solder a wire from pin 2 to the tip lug of the input jack.
 *     - Connect a wire from the VCC pin to the outer lug of the input jack.
 *   - **Expression Pedal:**
 *     - Connect the ground wire from the expression pedal to GND on the Arduino.
 *     - Connect the center lug wire to pin A0 (18) on the Arduino.
 *     - Connect the other wire to VCC on the Arduino. (You might need to swap the wire connections if it doesn't work correctly).
 *
 * @section Customization
 *   - To change the device name (e.g., "Amits Expresso Midi controller"), copy the `hardware` folder to your `Documents/Arduino` folder.
 *     If the folder already exists, copy the contents of the included `hardware` folder to your `Documents/Arduino/hardware` folder.
 *     Edit the `boards.txt` file to change the name.
 */

#include "ATPOTS.h"
#include <EEPROM.h>
#include <MIDI.h>
#include <USB-MIDI.h>

// Create a default USBMIDI instance.
USBMIDI_CREATE_DEFAULT_INSTANCE();

// ========== Pin Configuration ==========
#define pEXP 18 ///< Analog pin for the expression pedal (A0).
#define pSUSTAIN 2 ///< Digital pin for the sustain pedal.
#define blinker 9 ///< LED pin for potential tempo blink (not currently used).
#define MIDI_CH 11 ///< Default MIDI channel.
#define debounceMS 50 ///< Debounce time for the sustain pedal in milliseconds.

// ========== MIDI CC Configuration ==========
#define setEXP 33 ///< MIDI CC number to set the expression pedal's CC number.
#define setSustain 34 ///< MIDI CC number to set the sustain pedal's CC number.
#define pedalReset 35 ///< MIDI CC number to reset the pedal to default settings (even values).
#define pedalSave 36 ///< MIDI CC number to save the current configuration to EEPROM (value 127).
#define pedalLoad 37 ///< MIDI CC number to load the saved configuration from EEPROM (value 127).
#define pedalDeadZone 38 ///< MIDI CC number to set the dead zone for the expression pedal (values 1-50).
#define pedalExpCh 39 ///< MIDI CC number to set the MIDI output channel for the expression pedal (values 1-16).
#define pedalSustainCh 40 ///< MIDI CC number to set the MIDI output channel for the sustain pedal (values 1-16).

// ========== CC Value Limits ==========
#define setLOW 0 ///< Lower limit for settable CC values.
#define setHIGH 110 ///< Upper limit for settable CC values.

// ========== Default MIDI CC Assignments ==========
const byte SUSTAIN_CC = 64; ///< Default CC number for the sustain pedal.
const byte EXP_CC = 11; ///< Default CC number for the expression pedal.

// ========== Board Identifier ==========
const char ID[] = "ATEXPSO"; ///< Unique identifier for the board.

// ========== Default Dead Zone ==========
const float DEADZONE = 10.00f; ///< Default dead zone percentage for the expression pedal.

// ========== Global Variables ==========
byte ECC = EXP_CC; ///< Current CC number for the expression pedal.
byte SCC = SUSTAIN_CC; ///< Current CC number for the sustain pedal.
byte expCH = MIDI_CH; ///< Current MIDI channel for the expression pedal.
byte sustainCH = MIDI_CH; ///< Current MIDI channel for the sustain pedal.
byte lastState = LOW; ///< Last known state of the sustain pedal.
unsigned long lastScan = 0; ///< Timestamp of the last sustain pedal scan.
bool currentState = LOW; ///< Current state of the sustain pedal.

/**
 * @brief Structure to store the pedal's configuration settings.
 */
struct PEDALSTATE {
    byte ECC; ///< Expression pedal CC number.
    byte SCC; ///< Sustain pedal CC number.
    byte CH_EXPRESSION; ///< MIDI channel for the expression pedal.
    byte CH_SUSTAIN; ///< MIDI channel for the sustain pedal.
    float DEADZONE; ///< Dead zone percentage for the expression pedal.
    char ID[sizeof(ID)]; ///< Board identifier.
};

// Create an instance of the ATPOT class for the expression pedal.
ATPOT POT(pEXP, 0, 127, DEADZONE);

/**
 * @brief Handles incoming MIDI messages to configure the pedal.
 *
 * @details This function processes MIDI Control Change messages to:
 *          - Set the CC number for the expression pedal.
 *          - Set the CC number for the sustain pedal.
 *          - Reset the pedal to default settings.
 *          - Save the current configuration to EEPROM.
 *          - Load the saved configuration from EEPROM.
 *          - Set the dead zone for the expression pedal.
 *          - Set the MIDI channel for the expression pedal.
 *          - Set the MIDI channel for the sustain pedal.
 */
void handleMidiInput()
{
    if (!MIDI.read())
        return;

    if (MIDI.getType() != midi::ControlChange)
        return;

    byte data = constrain(MIDI.getData2(), setLOW, setHIGH);

    if (MIDI.getData1() == setEXP) {
        ECC = data;
        return;
    }
    if (MIDI.getData1() == setSustain) {
        SCC = data;
        return;
    }
    if (MIDI.getData1() == pedalReset) {
        if (data % 2 == 0)
            initPedal();
        return;
    }
    if (MIDI.getData1() == pedalSave && MIDI.getData2() == 127) {
        saveConfig();
        return;
    }
    if (MIDI.getData1() == pedalLoad && MIDI.getData2() == 127) {
        loadConfig();
        return;
    }
    if (MIDI.getData1() == pedalDeadZone) {
        POT.setDeadZone((float)constrain(MIDI.getData2(), 1, 50));
        return;
    }
    if (MIDI.getData1() == pedalExpCh) {
        expCH = constrain(MIDI.getData2(), 1, 16);
        return;
    }
    if (MIDI.getData1() == pedalSustainCh) {
        sustainCH = constrain(MIDI.getData2(), 1, 16);
        return;
    }
}

/**
 * @brief Initializes the pedal to its default settings.
 *
 * @details This function resets the expression and sustain pedal CC numbers,
 *          MIDI channels, and the dead zone to their default values.
 */
void initPedal()
{
    ECC = EXP_CC;
    SCC = SUSTAIN_CC;
    expCH = MIDI_CH;
    sustainCH = MIDI_CH;
    POT.setDeadZone(DEADZONE);
}

/**
 * @brief Arduino setup function.
 *
 * @details Initializes pin modes, serial communication, MIDI, and loads the configuration from EEPROM.
 */
void setup()
{
    pinMode(blinker, OUTPUT);
    pinMode(pSUSTAIN, INPUT_PULLUP);
    initPedal();

    digitalWrite(blinker, LOW);

    loadConfig();
    MIDI.begin(1); // Start MIDI on channel 1
    MIDI.turnThruOff();
}

/**
 * @brief Saves the current pedal configuration to EEPROM.
 *
 * @details Stores the expression pedal CC number, sustain pedal CC number,
 *          MIDI channels, dead zone, and board identifier in EEPROM.
 */
void saveConfig()
{
    PEDALSTATE P;
    P.ECC = ECC;
    P.SCC = SCC;
    P.CH_EXPRESSION = expCH;
    P.CH_SUSTAIN = sustainCH;
    P.DEADZONE = POT.getDeadZone();
    strcpy(P.ID, ID);
    EEPROM.put(0, P);
}

/**
 * @brief Loads the pedal configuration from EEPROM.
 *
 * @details Retrieves the expression pedal CC number, sustain pedal CC number,
 *          MIDI channels, dead zone, and board identifier from EEPROM.
 *          If no valid configuration is found, it sets the default values and saves them.
 */
void loadConfig()
{
    PEDALSTATE PS;
    EEPROM.get(0, PS);

    if (String(PS.ID) != String(ID)) { // eeprom does not contain initial state
        PS.ECC = EXP_CC;
        PS.SCC = SUSTAIN_CC;
        PS.CH_EXPRESSION = MIDI_CH;
        PS.CH_SUSTAIN = MIDI_CH;
        PS.DEADZONE = DEADZONE;
        strcpy(PS.ID, ID);
        EEPROM.put(0, PS);
    }

    ECC = PS.ECC;
    SCC = PS.SCC;
    expCH = PS.CH_EXPRESSION;
    sustainCH = PS.CH_SUSTAIN;
    POT.setDeadZone(PS.DEADZONE);
}

/**
 * @brief Handles the sustain pedal input.
 *
 * @details Reads the state of the sustain pedal and sends a MIDI CC message
 *          when the state changes. Implements debouncing to prevent spurious readings.
 */
void handleSustain()
{
    unsigned long now = millis();
    if ((now - lastScan) < debounceMS) {
        return;
    }

    byte state = digitalRead(pSUSTAIN);
    lastScan = now;
    if (state != lastState) {
        if (SCC)
            MIDI.sendControlChange(SCC, state == 1 ? 127 : 0, sustainCH);
        lastState = state;
    }
}

/**
 * @brief Arduino main loop function.
 *
 * @details Continuously scans the expression pedal, handles the sustain pedal,
 *          and processes incoming MIDI messages.
 */
void loop()
{
    POT.scan();
    if (POT.hasChanged) {
        if (ECC)
            MIDI.sendControlChange(ECC, POT.value, expCH);
        POT.reset();
    }
    handleSustain();
    handleMidiInput();
}
