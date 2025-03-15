/*****************************************************************************
 * Copyright © Amit Talwar www.amitszone.com
 * You are free to use this project for your own personal non commercial use.
 * by Using this project and code you agree and understand that you are
 * prohibited to build and sell this to others for profit.
 ******************************************************************************/
/*****************************************************************************
 * Header File for Amit's Potentiometer class for dealing with Potentiometers.
 *****************************************************************************/

#ifndef ATPOTS_H
#define ATPOTS_H // Corrected the macro name to be consistent
#define MAX_ANALOG_POT_READING 1023
#include <Arduino.h>

/**
 * @brief Represents a generic potentiometer connected to an analog pin.
 *
 * @details This class provides functionality to read, scale, and manage a potentiometer's
 *          analog input. It also includes support for a dead zone to compensate for
 *          low-precision potentiometers.
 */
class ATPOT {

public:
    /**
     * @brief Constructor for the ATPOT class with custom minimum and maximum values and a dead zone.
     *
     * @param pin The analog pin connected to the potentiometer.
     * @param minVal The minimum output value of the potentiometer.
     * @param maxVal The maximum output value of the potentiometer.
     * @param deadZonePercent The dead zone percentage to compensate for low-precision potentiometers (0.0 - 100.0).
     *
     * @details Initializes the potentiometer with custom minimum and maximum values, and a dead zone.
     *          The dead zone is calculated as a percentage of the total range (0-1023).
     */
    ATPOT(byte pin, int minVal, int maxVal, float deadZonePercent);

    /**
     * @brief Constructor for the ATPOT class with a dead zone and default minimum/maximum values.
     *
     * @param pin The analog pin connected to the potentiometer.
     * @param deadZonePercent The dead zone percentage to compensate for low-precision potentiometers (0.0 - 100.0).
     *
     * @details Initializes the potentiometer with a dead zone and default minimum (0) and maximum (1023) values.
     */
    ATPOT(byte pin, float deadZonePercent);

    /**
     * @brief Constructor for the ATPOT class with custom minimum and maximum values and a dead zone.
     *
     * @param pin The analog pin connected to the potentiometer.
     * @param minVal The minimum output value of the potentiometer.
     * @param maxVal The maximum output value of the potentiometer.
     * @param deadZonePercent The dead zone percentage to compensate for low-precision potentiometers (0.0 - 100.0).
     * @param handler function to handleChanged Event
     * @details Initializes the potentiometer with custom minimum and maximum values, and a dead zone.
     *          The dead zone is calculated as a percentage of the total range (0-1023).
     */
    ATPOT(byte pin, int minVal, int maxVal, float deadZonePercent, void (*handler)(byte, byte));

    /**
     * @brief Basic constructor for the ATPOT class with default minimum/maximum values and no dead zone.
     *
     * @param pin The analog pin connected to the potentiometer.
     *
     * @details Initializes the potentiometer with default minimum (0) and maximum (1023) values and no dead zone.
     */
    ATPOT(byte pin);

    /**
     * @brief Sets the number of readings to be averaged for the potentiometer.
     *
     * @param num The number of readings to average.
     *
     * @details This function sets the number of analog readings that will be taken and averaged
     *          in the `aRead()` function. A higher number of readings will result in a smoother,
     *          but potentially slower, response.
     */
    void setNumReadings(int num);

    /**
     * @brief Sets the debounce threshold for the potentiometer.
     *
     * @param threshold The debounce threshold value.
     *
     * @details This function sets the threshold used for debouncing in the `aRead()` function.
     *          The debounce threshold determines how much the reading must change before it is
     *          considered a significant change and not just noise.
     */
    void setDebounceThreshold(int threshold);

    /**
     * @brief Scans the potentiometer and updates its value.
     *
     * @details Reads the analog value, applies the dead zone, maps the value to the specified range,
     *          and triggers the `changed()` method if the value has changed.
     *          This function should be called repeatedly in the main loop to keep the potentiometer's
     *          state updated.
     */
    void scan();

    /**
     * @brief Sets the dead zone percentage for the potentiometer.
     *
     * @param deadZonePercent The new dead zone percentage (0.0 - 100.0).
     *
     * @details Updates the dead zone percentage and recalculates the dead zone factor.
     */
    void setDeadZone(float deadZonePercent);

    /**
     * @brief Gets the current dead zone percentage.
     *
     * @return The current dead zone percentage.
     */
    float getDeadZone() const; // Added const

    /**
     * @brief The raw analog value read from the potentiometer (0-1023).
     */
    int rawValue;

    /**
     * @brief The mapped value of the potentiometer, scaled to the range between `_minVal` and `_maxVal`.
     */
    int value;

    /**
     * @brief Flag indicating whether the potentiometer's value has changed since the last scan.
     */
    bool hasChanged = false;

    /**
     * @brief Resets the `hasChanged` flag.
     *
     * @details Sets the `hasChanged` flag to false. This should be called after processing a change.
     */
    void reset();
    /**
     * @brief Sets the change handler function.
     *
     * @param handler A pointer to the function to be called when the potentiometer's value changes.
     *
     * @details This function allows you to register a callback function that will be called
     *          whenever the potentiometer's value changes.
     */
    void setChangeHandler(void (*handler)(byte, byte));

protected:
    /**
     * @brief Virtual method called when the potentiometer's value changes.
     *
     * @details This method is called when the `scan()` function detects a change in the potentiometer's value.
     *          It can be overridden in derived classes to perform custom actions when the value changes.
     *          Sets the `hasChanged` flag to true.
     */
    virtual void changed(byte newValue, byte oldValue);

    /**
     * @brief The minimum output value of the potentiometer.
     */
    int _minVal = 0;

    /**
     * @brief The analog pin connected to the potentiometer.
     */
    byte _pin;

    /**
     * @brief The maximum output value of the potentiometer.
     */
    int _maxVal = MAX_ANALOG_POT_READING;

    /**
     * @brief The last read value of the potentiometer.
     */
    int _lastReading = -1; // Initialize to an invalid value

    /**
     * @brief The dead zone percentage.
     */
    float _deadZonePercent = 0;

    /**
     * @brief The calculated dead zone factor.
     */
    int _deadZoneFactor = 0;
    /**
     * @brief Pointer to the function that will be called when the potentiometer's value changes.
     */
    void (*_changeHandler)(byte, byte) = nullptr;

private:
    /**
     * @brief Reads the analog value from the potentiometer pin with averaging and debouncing.
     *
     * @return The averaged and debounced analog reading from the potentiometer.
     *
     * @details Reads the analog value from the specified pin multiple times, calculates the average,
     *          rejects outliers (highest and lowest values), and applies debouncing.
     *          Debouncing prevents small fluctuations in the reading from being registered as changes.
     */
    int aRead();

    /**
     * @brief The number of readings to be averaged.
     */
    int _numReadings = 10;
    /**
     * @brief The debounce threshold value.
     */
    int _debounceThreshold = 5; // Default debounce threshold
};

/**
 * @brief Represents a potentiometer that sends MIDI Control Change (CC) messages.
 *
 * @details This class inherits from `ATPOT` and adds functionality to send MIDI CC messages
 *          when the potentiometer's value changes. It can be configured to use a custom
 *          value array for non-linear mapping.
 */
class ATMIDICCPOT : public ATPOT {
public:
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
    ATMIDICCPOT(byte pin, byte ch, byte cc);
    /**
     * @brief Constructor for the ATMIDICCPOT class with custom dead zone.
     *
     * @param pin The analog pin connected to the potentiometer.
     * @param ch The MIDI channel (1-16).
     * @param cc The MIDI CC number.
     * @param deadZonePercent The dead zone percentage to compensate for low-precision potentiometers (0.0 - 100.0).
     *
     * @details Initializes the potentiometer with custom dead zone and default min/max values of 0 and 127.
     *          It also sets the MIDI channel and CC number.
     */
    ATMIDICCPOT(byte pin, byte ch, byte cc, float deadZonePercent);

    /**
     * @brief Initializes the ATMIDICCPOT with a MIDI channel and CC number.
     *
     * @param ch The MIDI channel (1-16).
     * @param cc The MIDI CC number.
     *
     * @details Initializes the MIDI channel and CC number for the potentiometer.
     *          The potentiometer will send MIDI CC messages with the specified channel and CC number.
     */
    void INIT(byte ch, byte cc);

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
    void INIT(byte ch, byte cc, byte* values, byte count);

    /**
     * @brief Overrides the `changed()` method to send MIDI CC messages.
     *
     * @details This method is called when the potentiometer's value changes.
     *          It sends a MIDI CC message with the assigned CC number and the mapped value.
     *          If a custom value array is used, the mapped index in the array is used as the value.
     */
    virtual void changed(byte newValue, byte oldValue);

private:
    /**
     * @brief The MIDI message type (Control Change).
     */
    byte _mesg;

    /**
     * @brief The MIDI CC number.
     */
    byte _cc;

    /**
     * @brief A pointer to a custom array of values to use for the potentiometer.
     */
    byte* _varr;

    /**
     * @brief The number of elements in the `_varr` array.
     */
    byte _count;

    /**
     * @brief Flag indicating whether a custom value array is used.
     */
    bool valueType = false;
};
#endif
