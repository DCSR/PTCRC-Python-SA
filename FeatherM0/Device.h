
/*
 * Device.h - Library for timed devices such as pumps, LEDs and food hoppers
 * Created by David Roberts, Mar 6, 2019
 */

#ifndef Device_h
#define Device_h 

#include "Arduino.h"

class Device  {
  public:
    Device(int num);
    void switchOn();
    void switchOff();
    void switchOnFor(long duration);
    void update();
  private:
    int _pin;      
    long _time;
    long _duration;
    boolean _timerOn = false;
};

#endif
