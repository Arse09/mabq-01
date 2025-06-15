/**
 * @file include/Sensors.h
 *
 * @author Arse09
 * @date 2025-06-11
 *
 * @copyright Copyright(c) 2025 Arse09
 *
 * @license MIT License
 *
 * GitHub Repository:
 * @link https://github.com/Arse09/mabq-01 @endlink
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Wire.h>

class IMU
{
  private:
    float rateRoll, ratePitch, rateYaw;
    float rateCalibrationRoll, rateCalibrationPitch, rateCalibrationYaw;
    int rateCalibrationNumber;
    bool setUp = false;

    void gyroRead()
    {
      Wire.beginTransmission(0x68);
      Wire.write(0x1A);
      Wire.write(0x05);
      Wire.endTransmission();

      Wire.beginTransmission(0x68);
      Wire.write(0x1B);
      Wire.write(0x8);
      Wire.endTransmission();

      Wire.beginTransmission(0x68);
      Wire.write(0x43);
      Wire.endTransmission();

      Wire.requestFrom(0x68, 6);

      int16_t gyroX = (Wire.read() << 8) | Wire.read();
      int16_t gyroY = (Wire.read() << 8) | Wire.read();
      int16_t gyroZ = (Wire.read() << 8) | Wire.read();

      rateRoll = gyroX / 65.5f;
      ratePitch = gyroY / 65.5f;
      rateYaw = gyroZ / 65.5f;
    }

  public:
    IMU(uint8_t sdaPin, uint8_t sclPin)
      : rateRoll(0), ratePitch(0), rateYaw(0), 
        rateCalibrationRoll(0), rateCalibrationPitch(0), rateCalibrationYaw(0)
    {
      Wire.setPins(sdaPin, sclPin);
    }

    float getRateRoll() const { return rateRoll; }
    float getRatePitch() const { return ratePitch; }
    float getRateYaw() const { return rateYaw; }

    void setup()
    {
      Wire.setClock(400000);
      Wire.begin();
      delay(250);
      Wire.beginTransmission(0x68);
      Wire.write(0x6B);
      Wire.write(0x00);
      Wire.endTransmission();

      for (rateCalibrationNumber = 0; rateCalibrationNumber < 2000; rateCalibrationNumber++)
      {
        gyroRead();
        rateCalibrationRoll += rateRoll;
        rateCalibrationPitch += ratePitch;
        rateCalibrationYaw += rateYaw;
        delay(1);
      }
      rateCalibrationRoll /= 2000.0f;
      rateCalibrationPitch /= 2000.0f;
      rateCalibrationYaw /= 2000.0f;

      setUp = true;
    }

    void loop()
    {
      if (!setUp)
      {
        return;
      }

      gyroRead();

      rateRoll -= rateCalibrationRoll;
      ratePitch -= rateCalibrationPitch;
      rateYaw -= rateCalibrationYaw;
    }
};


#endif
