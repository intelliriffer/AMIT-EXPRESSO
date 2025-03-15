/*****************************************************************************
 * Copyright © Amit Talwar www.amitszone.com
 * You are free to use this project for your own personal non commercial use. 
 * by Using this project and code you agree and understand that you are 
 * prohibited to build and sell this to others for profit.
******************************************************************************/
/*****************************************************************************
 * Header File for Amit's Potentiometer class for dealing with Potentiometers.
 *****************************************************************************/
 
#ifndef ATPOTS_h
#define ATPOS_h
#include <Arduino.h>
class ATPOT  {

  public:
    ATPOT(byte pin, int minVal, int maxVal, float deadZonePercent);
    ATPOT(byte pin, float deadZonePercent);
    ATPOT(byte pin);
    void scan();
    int rawValue;
    int value;
    bool hasChanged=false;
    void reset();
  protected:
    virtual  void changed();
    int potRead();
    int _minVal = 0;
    byte _pin;
    int _maxVal = 1023;
    int _lastReading;
    float _deadZonePercent = 0;
    int _deadZoneFactor = 0;

  private:
    int aRead();

};

class ATMIDICCPOT : public ATPOT {
  public:
    ATMIDICCPOT(byte pin, byte ch, byte cc);
    ATMIDICCPOT(byte pin, byte ch, byte cc, float deadZonePercent);
    void INIT(byte ch, byte cc);
    void INIT(byte ch, byte cc , byte *values, byte count);
    virtual void changed();
  private:
    byte _mesg;
    byte _cc;
    byte *_varr;
    byte _count;
    bool valueType = false;
};
#endif
