/**
 * @file src/Debug.cpp
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

#include "Debug.h"

namespace MABQ
{
  Debug::Debug(bool isEnabled, unsigned long baudRate)
    : enabled(isEnabled), baud(baudRate), serialBegan(false) {}

  void Debug::begin(unsigned long baudRate)
  {
    if (!enabled)
      return;

    if (!serialBegan)
    {
      Serial.begin(baud);
      serialBegan = true;
    }
  }
}