/**
 * @file include/Motors.h
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

#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>
#include <ESP32Servo.h>

class Motors
{
private:
  Servo motor1;
  Servo motor2;
  Servo motor3;
  Servo motor4;

  uint8_t pinMotor1, pinMotor2, pinMotor3, pinMotor4;

  float desiredThrottle;  // 0.0 a 1.0
  float desiredPitchRate; // º/s
  float desiredYawRate;   // º/s
  float desiredRollRate;  // º/s

  void writeMotors();

public:
  Motors(uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4);

  void setup();
  void loop();

  void setDesiredThrottle(float throttle); // 0.0 - 1.0
  void setDesiredPitchRate(float pitch);
  void setDesiredYawRate(float yaw);
  void setDesiredRollRate(float roll);
};

#endif
