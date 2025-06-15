/**
 * @file src/Receiver.cpp
 *
 * @author Arse09
 * @date 2025-06-12
 *
 * @copyright Copyright(c) 2025 Arse09
 *
 * @license MIT License
 *
 * GitHub Repository:6
 */

#include "Receiver.h"

Receiver *Receiver::instance = nullptr;

Receiver::Receiver()
{
  instance = this; // Guardamos puntero a esta instancia
}

void Receiver::setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.print(WiFi.macAddress());
  delay(3000);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error inicializando ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
}

void Receiver::onDataRecv(const uint8_t *mac, const uint8_t *data, int len)
{
  if (instance)
  {
    instance->handleData(data, len);
  }
}

#define CALIBRATION_SAMPLES 50

uint32_t sumThrottle = 0, sumYaw = 0, sumPitch = 0, sumRoll = 0;
uint8_t calibrationCount = 0;
bool calibrated = false;

void Receiver::handleData(const uint8_t *data, int len)
{
  if (len == sizeof(Payload))
  {
    Payload incoming;
    memcpy(&incoming, data, sizeof(Payload));

    if (!calibrated)
    {
      sumYaw += incoming.b;
      sumPitch += incoming.c;
      sumRoll += incoming.d;
      calibrationCount++;

      if (calibrationCount >= CALIBRATION_SAMPLES)
      {
        yawOffset = sumYaw / CALIBRATION_SAMPLES;
        pitchOffset = sumPitch / CALIBRATION_SAMPLES;
        rollOffset = sumRoll / CALIBRATION_SAMPLES;
        calibrated = true;

        Serial.printf("Calibrated offsets: Throttle=%d, Yaw=%d, Pitch=%d, Roll=%d\n",
                      throttleOffset, yawOffset, pitchOffset, rollOffset);
      }
      return; // mientras calibras no actualices rates
    }

    // Después de calibrar aplica la fórmula
    desiredThrottleRate = ((int)incoming.a) / 254.0f;

    desiredYawRate = ((int)incoming.b - yawOffset) / 127.0f * 10.0f;
    desiredPitchRate = ((int)incoming.c - pitchOffset) / 127.0f * 10.0f;
    desiredRollRate = ((int)incoming.d - rollOffset) / 127.0f * 10.0f;
  }
}

// Getters para acceder desde fuera

float Receiver::getDesiredPitchRate() const { return desiredPitchRate; }
float Receiver::getDesiredRollRate() const { return desiredRollRate; }
float Receiver::getDesiredYawRate() const { return desiredYawRate; }
float Receiver::getDesiredThrottleRate() const { return desiredThrottleRate; }
