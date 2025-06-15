/**
 * @file include/PwmLed.h
 *
 * @author Arse09
 * @date 2025-05-24
 *
 * @copyright Copyright(c) 2025 Arse09
 *
 * @license MIT License
 *
 * GitHub Repository:
 * @link https://github.com/Arse09/mabq-01 @endlink
 */

#ifndef PWMLED_H
#define PWMLED_H

#include <cstdint>
#include "Globals.h"

#include <ESP32PWM.h>

class PwmLed
{
private:
  const uint8_t pin;

  double brightness;

  ESP32PWM pwm;

public:
  /**
   * @param[in] pin
   * @param[in] brightnessLevel 0 to 100%
   */
  PwmLed(uint8_t pin, double initialBrightness = 0);

  void setup();

  /**
   * @param[in] brightnessLevel 0 to 100%
   */
  void setBrightness(double brightnessLevel);
};

#endif // PWMLED_H