/*****************************************************************************
 * Copyright © Amit Talwar www.amitszone.com
 * You are free to use this project for your own personal non commercial use. 
 * by Using this project and code you agree and understand that you are 
 * prohibited to build and sell this to others for profit.
******************************************************************************/
/*****************************************************************************
 * Amit's Potentiometer class for dealing with Potentiometers.
 *****************************************************************************/


#include "ATPOTS.h"
/* Basic Potentiometer assinged to a pin, triggered by value change, return value between 0-1023*/
ATPOT::ATPOT(byte pin) {
  _minVal = 0;
  _maxVal = 1023;
  _deadZonePercent = 0;
  _pin = pin;
  _deadZoneFactor = 1023 * _deadZonePercent / 100;
}


/* Basic Potentiometer assinged to a pin, triggered by value change, return value between 0-1023 with deadszone to deal with low precision/cheap potentiometers*/
ATPOT::ATPOT(byte pin, float deadZonePercent) {
  _minVal = 0;
  _maxVal = 1023;
  _deadZonePercent = deadZonePercent;
  _pin = pin;
  _deadZoneFactor = 1023 * _deadZonePercent / 100;

}

/* Basic Potentiometer assinged to a pin, triggered by value change, return value between minVal and MaxVal*/

ATPOT::ATPOT(byte pin, int minVal, int maxVal, float deadZonePercent) {
  _minVal = minVal;
  _maxVal = maxVal;
  _deadZonePercent = deadZonePercent;
  _pin = pin;
  _deadZoneFactor = 1023 * _deadZonePercent / 100;

}

int ATPOT::aRead() {
  int i;
  int rvalue = 0;
  int numReadings = 10;

  for (i = 0; i < numReadings; i++) {

    rvalue = rvalue + analogRead(_pin);

    delay(1);
  }
  rvalue = rvalue / numReadings;
  return rvalue;
}

/* This must be called in the main loop all the time to read the state of pins*/
void ATPOT::scan() {

  int val = aRead();
  
  val = constrain(map(val, _deadZoneFactor, 1023 - _deadZoneFactor, 0, 1023), 0, 1023);
      value = map(val, 0, 1023, _minVal, _maxVal);
  if (value !=_lastReading) {
    _lastReading = value;
    rawValue = val;
    changed();
  }
  //_lastReading = value;

}
void ATPOT::reset() {
  hasChanged=false;
  }
void ATPOT::changed() {
   hasChanged=true;
  //  Serial.println("base");
}

/*  Initialize Mehtod You will call these from your code mostly to change functions of the pot)
    *values is an array containing values to use for the pot, count is the size of that array.
*/
void ATMIDICCPOT::INIT(byte ch, byte cc , byte *values, byte count)  {

  _mesg = 175 + constrain(ch, 1, 16);
  this->_cc = cc;
  this->_count = count;
  
  _varr = values;

  this->valueType = true;
}

/* Initialize Mehtod You will call these from your code mostly to change functions of the pot) */
void ATMIDICCPOT::INIT(byte ch, byte cc ) {
  valueType = false;
  _mesg = 175 + constrain(ch, 1, 16);
  _cc = cc;
 // Serial.println ("pot::inited");
}


/* Basic Potentiometer assinged to a pin, triggered by value change, sends cc between 0-127, deadzone fixed to 1%*/
ATMIDICCPOT::ATMIDICCPOT(byte pin, byte ch, byte cc): ATPOT{pin, 0, 127, 1} {
  INIT(ch, cc);

}

void ATMIDICCPOT::changed() {
//  Serial.println("pot changed");
  byte _value = value;

  if (valueType) {
    byte index = map(value, 0, 127, 0, _count - 1);
    _value = _varr[index];
  }
  Serial.write(_mesg);
  Serial.write(_cc);
  Serial.write(constrain(_value, 0, 127));
}
