/**
 * @file src/StatusLed.cpp
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

#include "StatusLed.h"

StatusLed::StatusLed(uint8_t RedPin, uint8_t GreenPin, uint8_t BluePin)
    : leds{PwmLed(RedPin), PwmLed(GreenPin), PwmLed(BluePin)} {}

void StatusLed::setup(int pwmTimer)
{
  ESP32PWM::allocateTimer(pwmTimer);
  for (int i = 0; i < 3; ++i)
    leds[i].setup();
}

void StatusLed::loop()
{
  switch (activeMode)
  {
  case Mode::None:
    return;
    break;

  case Mode::Loading:
    static int directions[3] = {+2, 0, -2};
    static unsigned long loopCount = 0;
    loopCount++;

    for (int i = 0; i < 3; ++i)
    {
      if (values[i] == 0 && directions[i] == -2)
        directions[i] = +2;
      else if (values[i] == 254 && directions[i] == +2)
        directions[i] = -2;

      values[i] += directions[i];
    }
    break;
  }

  for (int i = 0; i < 3; ++i)
    leds[i].setBrightness((values[i] / 255.0f) * 100);
}

void StatusLed::resetMode()
{
  activeMode = Mode::None;

  values[0] = 0;
  values[1] = 0;
  values[2] = 0;

  for (int i = 0; i < 3; ++i)
    leds[i].setBrightness((values[i] / 255.0f) * 100);
}

void StatusLed::setMode(Mode mode)
{
  activeMode = mode;

  switch (activeMode)
  {
  case Mode::None:
    return;
    break;

  case Mode::Loading:
    values[0] = 0;
    values[1] = 0;
    values[2] = 254;
    break;
  }

  for (int i = 0; i < 3; ++i)
    leds[i].setBrightness((values[i] / 255.0f) * 100);
}
