/**
 * @file src/PwmLed.cpp
 *
 * @author Arse09
 * @date 2025-05-25
 *
 * @copyright Copyright(c) 2025 Arse09
 *
 * @license MIT License
 *
 * GitHub Repository:
 * @link https://github.com/Arse09/mabq-01 @endlink
 */

#include "PwmLed.h"

PwmLed::PwmLed(uint8_t pin, double initialBrightness)
    : pin(pin), brightness(initialBrightness) {}

void PwmLed::setup()
{
  pwm.attachPin(pin, 10000, 8);
}

void PwmLed::setBrightness(double brightnessLevel)
{
  if (brightnessLevel > 100)
    return;

  pwm.writeScaled(brightnessLevel / 100.0f);
}