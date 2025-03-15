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
ATPOT::ATPOT(byte pin, int minVal, int maxVal, float deadZonePercent, void (*handler)(byte, byte))
{
    _minVal = minVal;
    _maxVal = maxVal;
    _deadZonePercent = deadZonePercent;
    _pin = pin;
    _deadZoneFactor = MAX_ANALOG_POT_READING * _deadZonePercent / 100;
    setChangeHandler(handler);
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
 * @brief Sets the number of readings to be averaged for the potentiometer.
 *
 * @param num The number of readings to average.
 *
 * @details This function sets the number of analog readings that will be taken and averaged
 *          in the `aRead()` function. A higher number of readings will result in a smoother,
 *          but potentially slower, response.
 */
void ATPOT::setNumReadings(int num)
{
    _numReadings = num;
}

/**
 * @brief Sets the debounce threshold for the potentiometer.
 *
 * @param threshold The debounce threshold value.
 *
 * @details This function sets the threshold used for debouncing in the `aRead()` function.
 *          The debounce threshold determines how much the reading must change before it is
 *          considered a significant change and not just noise.
 */
void ATPOT::setDebounceThreshold(int threshold)
{
    _debounceThreshold = threshold;
}

/**
 * @brief Reads the analog value from the potentiometer pin with averaging and debouncing.
 *
 * @return The averaged and debounced analog reading from the potentiometer.
 *
 * @details Reads the analog value from the specified pin multiple times, calculates the average,
 *          rejects outliers (highest and lowest values), and applies debouncing.
 *          Debouncing prevents small fluctuations in the reading from being registered as changes.
 */
int ATPOT::aRead()
{
    // debouncing was improved from gemini suggestions.
    unsigned long startTime = millis();
    int readings[_numReadings]; // Array to store readings
    int total = 0;
    int minVal = 1024; // Initialize to a value greater than the max possible reading
    int maxVal = -1; // Initialize to a value less than the min possible reading
    static int lastAverage = 0; // Static variable to remember the last average

    for (int i = 0; i < _numReadings; i++) {
        readings[i] = analogRead(_pin);
        total += readings[i];
        if (readings[i] < minVal) {
            minVal = readings[i];
        }
        if (readings[i] > maxVal) {
            maxVal = readings[i];
        }
    }

    // Remove the highest and lowest values (outlier rejection)
    total -= (minVal + maxVal);

    // Calculate the average of the remaining readings
    int currentAverage = total / (_numReadings - 2);

    // Debouncing: Check if the change is significant
    if (abs(currentAverage - lastAverage) < _debounceThreshold) {
        // Change is too small, consider it noise, return the last average
        return lastAverage;
    } else {
        // Significant change, update the last average and return the new average
        lastAverage = currentAverage;
        return currentAverage;
    }
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
    int newValue = map(val, 0, MAX_ANALOG_POT_READING, _minVal, _maxVal);
    if (newValue != _lastReading) {
        byte oldVal = _lastReading;
        _lastReading = newValue;
        rawValue = val;
        changed(newValue, oldVal);
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
void ATPOT::changed(byte newValue, byte oldValue)
{
    hasChanged = true;
    //  Serial.println("base");
    if (_changeHandler != nullptr) {
        _changeHandler(newValue, oldValue); // Call the registered handler
        reset(); // reset state as its been handled
    }
}

/**
 * @brief Sets the change handler function.
 *
 * @param handler A pointer to the function to be called when the potentiometer's value changes.
 *
 * @details This function allows you to register a callback function that will be called
 *          whenever the potentiometer's value changes.
 */
void ATPOT::setChangeHandler(void (*handler)(byte, byte))
{
    _changeHandler = handler;
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
 *          This allows for non-linear mapping of the potentiometer's position to MIDI values.
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
 *          The MIDI message is sent over the hardware serial port.
 */
void ATMIDICCPOT::changed(byte newValue, byte oldValue)
{
    //  Serial.println("pot changed");
    byte _value = newValue;

    if (valueType) {
        byte index = map(newValue, 0, 127, 0, _count - 1);
        _value = _varr[index];
    }
    Serial.write(_mesg);
    Serial.write(_cc);
    Serial.write(constrain(_value, 0, 127));

    if (_changeHandler != nullptr) {
        _changeHandler(_value, oldValue); // Call the registered handler
        reset();
    }
}