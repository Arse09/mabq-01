/**
 * @file src/Motors.cpp
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

#include "Motors.h"
#include <ESP32PWM.h>

Motors::Motors(uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4)
    : pinMotor1(m1), pinMotor2(m2), pinMotor3(m3), pinMotor4(m4),
      desiredThrottle(0), desiredPitchRate(0), desiredYawRate(0), desiredRollRate(0)
{
}

void Motors::setup()
{
  ESP32PWM::allocateTimer(3);

  motor1.setPeriodHertz(50); // 50Hz para ESC
  motor2.setPeriodHertz(50);
  motor3.setPeriodHertz(50);
  motor4.setPeriodHertz(50);

  motor1.attach(pinMotor1, 1000, 2000);
  motor2.attach(pinMotor2, 1000, 2000);
  motor3.attach(pinMotor3, 1000, 2000);
  motor4.attach(pinMotor4, 1000, 2000);

  // Calibración ESC - enviar pulso máximo (throttle up)
  Serial.println("Calibrando ESC: enviar pulso máximo");
  motor1.writeMicroseconds(2000);
  motor2.writeMicroseconds(2000);
  motor3.writeMicroseconds(2000);
  motor4.writeMicroseconds(2000);
  delay(2000); // Esperar 2 segundos

  // Luego pulso mínimo (throttle down)
  Serial.println("Calibrando ESC: enviar pulso mínimo");
  motor1.writeMicroseconds(1000);
  motor2.writeMicroseconds(1000);
  motor3.writeMicroseconds(1000);
  motor4.writeMicroseconds(1000);
  delay(2000); // Esperar 2 segundos

  Serial.println("Calibración ESC finalizada");
}

void Motors::loop()
{
  // Aquí irá la lógica de mezcla y control PID si quieres

  writeMotors();
}

void Motors::writeMotors()
{
  // Supongamos desiredThrottle en [0..1]
  // desiredPitchRate, desiredRollRate, desiredYawRate en º/s

  // Coeficientes de ganancia para convertir tasas a valores PWM ajustables
  // (puedes calibrar estos valores a tus motores/ESC)
  const float pitchGain = 5.0f;
  const float rollGain = 5.0f;
  const float yawGain = 3.0f;

  // Calculamos la señal combinada para cada motor (valores en rango libre)
  float m1 = desiredThrottle + pitchGain * desiredPitchRate + rollGain * desiredRollRate - yawGain * desiredYawRate;
  float m2 = desiredThrottle + pitchGain * desiredPitchRate - rollGain * desiredRollRate + yawGain * desiredYawRate;
  float m3 = desiredThrottle - pitchGain * desiredPitchRate + rollGain * desiredRollRate + yawGain * desiredYawRate;
  float m4 = desiredThrottle - pitchGain * desiredPitchRate - rollGain * desiredRollRate - yawGain * desiredYawRate;

  // Limitamos cada motor al rango [0,1]
  m1 = constrain(m1, 0.0f, 1.0f);
  m2 = constrain(m2, 0.0f, 1.0f);
  m3 = constrain(m3, 0.0f, 1.0f);       
  m4 = constrain(m4, 0.0f, 1.0f);

  // Convertir a microsegundos (pulso ESC)
  int pulse1 = map((int)(m1 * 255), 0, 255, 1055, 2000);
  int pulse2 = map((int)(m2 * 255), 0, 255, 1055, 2000);
  int pulse3 = map((int)(m3 * 255), 0, 255, 1055, 2000);
  int pulse4 = map((int)(m4 * 255), 0, 255, 1055, 2000);

  motor1.writeMicroseconds(pulse1);
  motor2.writeMicroseconds(pulse2);
  motor3.writeMicroseconds(pulse3);
  motor4.writeMicroseconds(pulse4);
}

void Motors::setDesiredThrottle(float throttle)
{
  desiredThrottle = constrain(throttle, 0.0f, 1.0f);
}

void Motors::setDesiredPitchRate(float pitch)
{
  desiredPitchRate = pitch;
}

void Motors::setDesiredYawRate(float yaw)
{
  desiredYawRate = yaw;
}

void Motors::setDesiredRollRate(float roll)
{
  desiredRollRate = roll;
}
