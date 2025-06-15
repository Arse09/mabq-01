/**
 * @file include/Debug.h
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

#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

namespace MABQ
{
  class Debug
  {
  public:
    const bool enabled;
    const unsigned long baud;

  private:
    bool serialBegan = false;

    void printAll();

    template <typename T, typename... Args>
    void printAll(const T &first, const Args &...rest);

  public:
    Debug(bool isEnabled, unsigned long baudRate = 115200);

    void begin(unsigned long baudRate = 115200);

    template <typename... Args>
    void log(const Args &...args);
  };
}

// ----------- Template implementations ------------

inline void MABQ::Debug::printAll() {}

template <typename T, typename... Args>
void MABQ::Debug::printAll(const T &first, const Args &...rest)
{
  Serial.print(first);
  if (sizeof...(rest) > 0)
  {
    Serial.print(' ');
    printAll(rest...);
  }
}

template <typename... Args>
void MABQ::Debug::log(const Args &...args)
{
  if (!enabled || !serialBegan)
    return;

  printAll(args...);
  Serial.println();
}

#endif // DEBUG_H