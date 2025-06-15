/**
 * @file src/main.cpp
 *
 * @author Arse09
 * @date 2025-05-23
 *
 * @copyright Copyright(c) 2025 Arse09
 *
 * @license MIT License
 *
 * GitHub Repository:
 * @link https://github.com/Arse09/mabq-01 @endlink
 */

#include <Arduino.h>
#include <Wire.h>

#define DEBUG_ENABLED true
#include "Debug.h"

#include "Globals.h"

#include "ArmsLed.h"
#include "StatusLed.h"
#include "Program.h"
#include "Sensors.h"
#include "Receiver.h"
#include "Motors.h"

using namespace MABQ;

Debug debug(/*enabled=*/true, /*baud=*/115200);
Program *MABQuad01 = newProgram();

/* --------- ARMS' LED PINOUT --------- */
ArmsLed armsLed(18, 16, 32, 13);  // TL, TR, BL, BR led pins

/* ------ RGB STATUS LED PINOUT ------- */
StatusLed statusLed(2, 4, 15);    // Red, Green, blue led pins

/* -------------- TASKS --------------- */
Program::Task *task0 = MABQuad01->newTask(/* id = */ 0, /* hasOneLife = */ true);

Program::Task *task1 = MABQuad01->newTask(/* id = */ 1, /* hasOneLife = */ false);

/* ------------- SENSORS -------------- */
IMU imu(/*  */ 21, 22);

/* ------------- RECEIVER ------------- */
Receiver receiver;

/* -------------- MOTORS -------------- */
Motors motors(
    /* motor1 = */ 5,
    /* motor2 = */ 17,
    /* motor3 = */ 19,
    /* motor4 = */ 18 
);

void setup()
{
  MABQ::gDebug = &debug;
  debug.begin();

  /* -------------- TASK 0 -------------- */
  // Task 0: onSetup
  task0->onSetup = [](Program::Task *t)
  {
    debug.log("Setup t0");

    armsLed.setup(0);   // Pwm timer
    statusLed.setup(1); // Pwm timer

    t->transitionTo(1);
  };

  /* -------------- TASK 1 -------------- */
  // Task 1: onSetup
  task1->onSetup = [](Program::Task *t)
  {
    debug.log("Setup t1");
    
    imu.setup();
    receiver.setup();
    motors.setup();
  };
  // Task 1: onLoop
  task1->onLoop = [](Program::Task *t)
  {
    // debug.log("Loop t1");
    motors.setDesiredThrottle(receiver.getDesiredThrottleRate());
    motors.setDesiredYawRate(receiver.getDesiredYawRate());
    motors.setDesiredPitchRate(receiver.getDesiredPitchRate());
    motors.setDesiredRollRate(receiver.getDesiredRollRate());
  };

  MABQuad01->setup();
}

void loop()
{

  MABQuad01->loop();

  armsLed.loop();
  statusLed.loop();

  imu.loop();

  motors.loop();

  // debug
  debug.log("Desired roll: ", receiver.getDesiredRollRate(),
            " | Desired pitch: ", receiver.getDesiredPitchRate(),
            " | Desired yaw: ", receiver.getDesiredYawRate(),
            " | Desired throttle: ", receiver.getDesiredThrottleRate());

  delay(5);
}
