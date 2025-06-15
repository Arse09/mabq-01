/**
 * @file include/ArmsLed.h
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

#ifndef ARMSLED_H
#define ARMSLED_H

#include <cstdint>
#include <string>
#include "Globals.h"

#include "PwmLed.h"

class ArmsLed
{
private:
  PwmLed leds[4];

  int brightnesses[4] = {0, 0, 0, 0};

  std::string activeMode = "none";

public:
  ArmsLed(uint8_t TLPin, uint8_t TRPin, uint8_t BLPin, uint8_t BRPin);

  void setup(int pwmTimer);
  void loop();
  void resetMode();
  void setMode(std::string mode);
};

#endif // ARMSLED_H