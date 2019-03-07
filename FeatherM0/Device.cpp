/*
 * Device.cpp - Library for timed devices such as pumps, LEDs and food hoppers
 * Created by David Roberts, Mar 6, 2019
 */

 #include "Arduino.h"
 #include "Device.h"

void switchPin(int pin, int state) {
  if (state == 0){
      // pixel.setPixelColor(pin, pixel.Color(0,0,0));
      // pixel.show();
      // delay(1);
  }
  else {
      // pixel.setPixelColor(pin, pixel.Color(50,50,50));
      // pixel.show();
      // delay(1);
  }
}
 

 Device::Device(int num) {
  _pin = num;
}

void Device::switchOn() {
  switchPin(_pin, 1);
  _timerOn = false;
  _time = 0;
}

void Device::switchOff() {
  switchPin(_pin, 0);
  _timerOn = false;
  _time = 0;
}

void Device::switchOnFor(long duration) {
  switchPin(_pin, 1);
  _time = 0;
  _duration = duration;
  _timerOn = true;
}

void Device::update() {
  if (_timerOn) {
    _time++;
    if (_time == _duration) {
      _timerOn = false;
      _time = 0;
      switchPin(_pin, 0);
    }
  }
}

