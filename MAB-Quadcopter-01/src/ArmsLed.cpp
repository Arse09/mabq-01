/**
 * @file src/ArmsLed.cpp
 *
 * @author Arse09
 * @date 2025-05-26
 *
 * @copyright Copyright(c) 2025 Arse09
 *
 * @license MIT License
 *
 * GitHub Repository:
 * @link https://github.com/Arse09/mabq-01 @endlink
 */

#include "ArmsLed.h"

ArmsLed::ArmsLed(uint8_t TLPin, uint8_t TRPin, uint8_t BLPin, uint8_t BRPin)
    : leds{PwmLed(TLPin), PwmLed(TRPin), PwmLed(BLPin), PwmLed(BRPin)} {}

void ArmsLed::setup(int pwmTimer)
{
  ESP32PWM::allocateTimer(pwmTimer);
  for (int i = 0; i < 4; ++i)
    leds[i].setup();
}

void ArmsLed::loop()
{
  if (activeMode == "none")
    return;
}

void ArmsLed::resetMode()
{
  activeMode = "none";

  brightnesses[0] = 0;
  brightnesses[1] = 0;
  brightnesses[2] = 0;
  brightnesses[3] = 0;
}

void ArmsLed::setMode(std::string mode)
{
  activeMode = mode;
  
  if (false) { }
  else 
  {
    brightnesses[0] = 0;
    brightnesses[1] = 0;
    brightnesses[2] = 0;
    brightnesses[3] = 0;
  }
}
