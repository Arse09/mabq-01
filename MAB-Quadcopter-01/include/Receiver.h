/**
 * @file include/Receiver.h
 *
 * @author Arse09
 * @date 2025-06-12
 *
 * @copyright Copyright(c) 2025 Arse09
 *
 * @license MIT License
 *
 * GitHub Repository:
 * @link https://github.com/Arse09/mabq-01 @endlink
 */

#ifndef RECEIVER_H
#define RECEIVER_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

typedef struct
{
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d;
} Payload;

class Receiver
{
private:
  float desiredThrottleRate,
      desiredYawRate,
      desiredPitchRate,
      desiredRollRate;

  float throttleOffset, 
        yawOffset, 
        pitchOffset, 
        rollOffset;

  static Receiver *instance; // Puntero singleton para acceder en static callback

  static void onDataRecv(const uint8_t *mac, const uint8_t *data, int len);
  void handleData(const uint8_t *data, int len); // Método no estático

public:
  Receiver();
  void setup();

  // Métodos para obtener los valores
  float getDesiredThrottleRate() const;
  float getDesiredYawRate() const;
  float getDesiredPitchRate() const;
  float getDesiredRollRate() const;
};

#endif
