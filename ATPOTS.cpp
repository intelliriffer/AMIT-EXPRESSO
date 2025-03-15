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

/**
 * @brief Constructor for the ATPOT class.
 *
 * @param pin The analog pin connected to the potentiometer.
 * @param minVal The minimum output value of the potentiometer.
 * @param maxVal The maximum output value of the potentiometer.
 * @param deadZonePercent The dead zone percentage to compensate for low-precision potentiometers.
 *
 * @details Initializes the potentiometer with custom minimum and maximum values, and a dead zone.
 *          The dead zone is calculated as a percentage of the total range (0-1023).
 */
ATPOT::ATPOT(byte pin, int minVal, int maxVal, float deadZonePercent)
{
    _minVal = minVal;
    _maxVal = maxVal;
    _deadZonePercent = deadZonePercent;
    _pin = pin;
    _deadZoneFactor = MAX_ANALOG_POT_READING * _deadZonePercent / 100;
}

/**
 * @brief Constructor for the ATPOT class with a dead zone.
 *
 * @param pin The analog pin connected to the potentiometer.
 * @param deadZonePercent The dead zone percentage to compensate for low-precision potentiometers.
 *
 * @details Initializes the potentiometer with a dead zone and default minimum (0) and maximum (1023) values.
 */
ATPOT::ATPOT(byte pin, float deadZonePercent)
{
    _minVal = 0;
    _maxVal = MAX_ANALOG_POT_READING;
    _deadZonePercent = deadZonePercent;
    _pin = pin;
    _deadZoneFactor = MAX_ANALOG_POT_READING * _deadZonePercent / 100;
}

/**
 * @brief Basic constructor for the ATPOT class.
 *
 * @param pin The analog pin connected to the potentiometer.
 *
 * @details Initializes the potentiometer with default minimum (0) and maximum (1023) values and no dead zone.
 */
ATPOT::ATPOT(byte pin)
{
    _minVal = 0;
    _maxVal = MAX_ANALOG_POT_READING;
    _deadZonePercent = 0;
    _pin = pin;
    _deadZoneFactor = MAX_ANALOG_POT_READING * _deadZonePercent / 100;
}

/**
 * @brief Reads the analog value from the potentiometer pin with averaging.
 *
 * @return The averaged analog reading from the potentiometer.
 *
 * @details Reads the analog value from the specified pin multiple times and returns the average.
 *          This helps to reduce noise and improve the accuracy of the reading.
 */
int ATPOT::aRead()
{
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

/**
 * @brief Scans the potentiometer and updates its value.
 *
 * @details Reads the analog value, applies the dead zone, maps the value to the specified range,
 *          and triggers the `changed()` method if the value has changed.
 *          This function should be called repeatedly in the main loop to keep the potentiometer's
 *          state updated.
 */
void ATPOT::scan()
{

    int val = aRead();

    val = constrain(map(val, _deadZoneFactor, MAX_ANALOG_POT_READING - _deadZoneFactor, 0, MAX_ANALOG_POT_READING), 0, MAX_ANALOG_POT_READING);
    value = map(val, 0, MAX_ANALOG_POT_READING, _minVal, _maxVal);
    if (value != _lastReading) {
        _lastReading = value;
        rawValue = val;
        changed();
    }
    //_lastReading = value;
}

/**
 * @brief Resets the `hasChanged` flag.
 *
 * @details Sets the `hasChanged` flag to false. This should be called after processing a change.
 */
void ATPOT::reset()
{
    hasChanged = false;
}

/**
 * @brief Virtual method called when the potentiometer's value changes.
 *
 * @details This method is called when the `scan()` function detects a change in the potentiometer's value.
 *          It can be overridden in derived classes to perform custom actions when the value changes.
 *          Sets the `hasChanged` flag to true.
 */
void ATPOT::changed()
{
    hasChanged = true;
    //  Serial.println("base");
}

/**
 * @brief Sets the dead zone percentage for the potentiometer.
 *
 * @param deadZonePercent The new dead zone percentage.
 *
 * @details Updates the dead zone percentage and recalculates the dead zone factor.
 */
void ATPOT::setDeadZone(float deadZonePercent)
{
    _deadZonePercent = deadZonePercent;
    _deadZoneFactor = MAX_ANALOG_POT_READING * _deadZonePercent / 100;
}

/**
 * @brief Gets the current dead zone percentage.
 *
 * @return The current dead zone percentage.
 */
float ATPOT::getDeadZone() const
{
    return _deadZonePercent;
}

/**
 * @brief Initializes the ATMIDICCPOT with a custom value array.
 *
 * @param ch The MIDI channel (1-16).
 * @param cc The MIDI CC number.
 * @param values A pointer to an array of byte values to use for the potentiometer.
 * @param count The number of elements in the `values` array.
 *
 * @details This method initializes the MIDI channel, CC number, and a custom value array.
 *          When the potentiometer's value changes, the mapped index in the `values` array will be used.
 */
void ATMIDICCPOT::INIT(byte ch, byte cc, byte* values, byte count)
{

    _mesg = 175 + constrain(ch, 1, 16);
    this->_cc = cc;
    this->_count = count;

    _varr = values;

    this->valueType = true;
}

/**
 * @brief Initializes the ATMIDICCPOT with a MIDI channel and CC number.
 *
 * @param ch The MIDI channel (1-16).
 * @param cc The MIDI CC number.
 *
 * @details Initializes the MIDI channel and CC number for the potentiometer.
 *          The potentiometer will send MIDI CC messages with the specified channel and CC number.
 */
void ATMIDICCPOT::INIT(byte ch, byte cc)
{
    valueType = false;
    _mesg = 175 + constrain(ch, 1, 16);
    _cc = cc;
    // Serial.println ("pot::inited");
}

/**
 * @brief Constructor for the ATMIDICCPOT class.
 *
 * @param pin The analog pin connected to the potentiometer.
 * @param ch The MIDI channel (1-16).
 * @param cc The MIDI CC number.
 *
 * @details Initializes the potentiometer with a fixed dead zone of 1% and default min/max values of 0 and 127.
 *          It also sets the MIDI channel and CC number.
 */
ATMIDICCPOT::ATMIDICCPOT(byte pin, byte ch, byte cc)
    : ATPOT { pin, 0, 127, 1 }
{
    INIT(ch, cc);
}

/**
 * @brief Overrides the `changed()` method to send MIDI CC messages.
 *
 * @details This method is called when the potentiometer's value changes.
 *          It sends a MIDI CC message with the assigned CC number and the mapped value.
 *          If a custom value array is used, the mapped index in the array is used as the value.
 */
void ATMIDICCPOT::changed()
{
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
