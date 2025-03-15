
/**
Author: Amit Talwar.
License : Do whatever the F*** you want for Personal Use, but selling it to others for Profit is prohibited!
Board Used : Arduino Micro Pro (Leonardo)
Description: Expression pedal to Programmable Usb Midi Pedal Converter with Damper/Sustain Pedal / Switch Inpute

Components Requirements:

1: An Expression pedal To Convert (I used a cheap nektar  Expression Peaal as it plastic body is easier to burn hole in for sustin pedal input jack.   )

2: A Mono Input jack (for sustain pedal)
3: 10k resistor
4: Connection wires
5: Arduino Micro 

Wiring:
1: Solder a 10k resistor between pin 2 and Ground (gnd). on Arduino
2: Solder a wire to pin 2 of board to tip lig of Input Jack.
3: connect a wire from vcc pin to outer lug or Input Jack.
4: Your expression pedal should have three wires or connectors (you might as well want to bypass the polarity switch).
  connect the black or ground wire from your expression pedal ports to gnd on arduino.
  connect the center lug wire to pin A0 (18) on Arduino
  Conenct the Other wire to vcc on arduino. (you might have to swap the wire connection if these does not work correctly);

You Device should appear as Arduino Leonardo in your Device. however you can use custom ID and identifiers if you so desire..
by copying the hardware Folder in your Documents/Arduino Folder and restart Arduino IDE and choosintg Amits Expresso as Board for Connected Device.

**/

#include <MIDI.h>
#include <USB-MIDI.h>
#include "ATPOTS.h"
#include <EEPROM.h>



USBMIDI_CREATE_DEFAULT_INSTANCE();

// ========== Pin Configuration ==========
#define pEXP 18  //A0 19 20 21
#define pSUSTAIN 2
#define blinker 9  // led pinf for tempo blink led
#define MIDI_CH 1
#define debounceMS 50
#define setEXP 33      //value of cc 13 will set what expression pedal controls value of zero disables function
#define setSustain 34  //value of cc 15 controls what damper switch will control; value of 0 disables!
#define pedalReset 35  // even values will set pedal to default values;
#define pedalSave 36   // value of 127 will save current config to pedal.
#define pedalLoad 37   // value of 127 will load Saved config from pedal!
#define setLOW 0
#define setHIGH 110  //limit cc setup within these values.
#define pedalDeadZone 10 //if your pedal has issues going to absolute zero or full values a 10%, tweaking deadzome might bring it in usable proximity.


ATPOT POT(pEXP, 0, 127, 10);

// ========== MIDI Configuration ==========
const byte SUSTAIN_CC = 64;
const byte EXP_CC = 11;
const char ID[] = "AEXPR";
byte ECC = EXP_CC;
byte SCC = SUSTAIN_CC;

byte lastState = LOW;
unsigned long lastScan = 0;
bool currentState = LOW;


struct PEDALSTATE {
  byte ECC;
  byte SCC;
  char ID[sizeof(ID)];
};


void handleMidiInput() {

  if (MIDI.read()) {
    // Serial.print("MIDI Type: ");
    // Serial.print(MIDI.getType());
    // Serial.print(" | Data1: ");
    // Serial.print(MIDI.getData1());
    // Serial.print(" | Data2: ");
    // Serial.println(MIDI.getData2());

    if (MIDI.getType() == midi::ControlChange) {
      byte data = constrain(MIDI.getData2(), setLOW, setHIGH);

      if (MIDI.getData1() == setEXP) {
        ECC = data;
      }
      if (MIDI.getData1() == setSustain) {
        SCC = data;
      }
      if (MIDI.getData1() == pedalReset) {
        if (data % 2 == 0)
          initPedal();
      }
      if (MIDI.getData1() == pedalSave && MIDI.getData2() == 127) {
        saveConfig();
      }
      if (MIDI.getData1() == pedalLoad && MIDI.getData2()== 127) {
        loadConfig();
      }
    }
  }
}

void initPedal() {  //reset pedal to defaults
  ECC = EXP_CC;
  SCC = SUSTAIN_CC;
}

void setup() {
  pinMode(blinker, OUTPUT);
  pinMode(pSUSTAIN, INPUT_PULLUP);
  initPedal();

  // pinMode(SW_PIN, INPUT_PULLUP);  // Rotary Encoder Button
  digitalWrite(blinker, LOW);
  // Serial.begin(115200);  // Debugging output
  // while (!Serial) {
  //   ;  // wait for serial port to connect. Needed for native USB port only
  // }
  loadConfig();
  MIDI.begin(1);  // Start MIDI on channel 1
  MIDI.turnThruOff();
}
void saveConfig() {
  PEDALSTATE P;
  P.ECC = ECC;
  P.SCC = SCC;
  strcpy(P.ID, ID);
  EEPROM.put(0, P);
}
void loadConfig() {
  PEDALSTATE PS;
  EEPROM.get(0, PS);
  // Serial.println("Checking Saved State");
  if (String(PS.ID) != String(ID)) {
    // Serial.println("ID Not found");
    PS.ECC = EXP_CC;
    PS.SCC = SUSTAIN_CC;
    
    // Serial.println(String(PS.ID));
    strcpy(PS.ID, ID);
    // Serial.println(String(PS.ID));
    EEPROM.put(0, PS);
  } 
  // else {
  //   // Serial.println("Loaded Values from EEPROM");
  //   Serial.println(PS.ECC);
  //   Serial.println(PS.SCC);
  //   Serial.println(PS.ID);
  // }
  ECC = PS.ECC;
  SCC = PS.SCC;
}


void handleSustain() {
  unsigned long now = millis();
  if ((now - lastScan) < debounceMS) {
    return;
  }

  //  Serial.print("Reading Button with currentState: ");
  byte state = digitalRead(pSUSTAIN);
  // Serial.println(state);
  //  Serial.println(lastState);
  lastScan = now;
  if (state != lastState) {
    // Serial.print("State Changed from: ");
    // Serial.print(lastState);
    // Serial.print(" to: ");
    // Serial.println(state);
    if (SCC)
      MIDI.sendControlChange(SCC, state == 1 ? 127 : 0, MIDI_CH);
    lastState = state;
  }
}



void loop() {

  POT.scan();
  if (POT.hasChanged) {
    // Serial.print("Pot has Value of: ");
    if (ECC)
      MIDI.sendControlChange(ECC, POT.value, MIDI_CH);
    //Serial.println(POT.value);
    POT.reset();
  }
  handleSustain();
  handleMidiInput();
  //handleEncoderSwitch();
}
