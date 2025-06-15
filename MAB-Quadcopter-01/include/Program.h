/**
 * @file include/Program.h
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

#ifndef PROGRAM_H
#define PROGRAM_H

#include <functional>
#include <vector>
#include <string>
#include "Globals.h"

namespace MABQ
{
  class Program
  {
  public:
    class Task
    {
    public:
      const uint8_t id;
      bool settingUp;
      unsigned long loopCount;
      bool exiting;
      const bool oneLife;

      void transitionTo(uint8_t id);

      std::function<void(Task *)> onSetup;
      std::function<void(Task *)> onLoop;

    private:
      Program *parentProgram;

      Task(uint8_t id, bool hasOneLife, Program *parent);

      void setup();
      void loop();
      void exit();

      friend class Program;
    };

    private:
    Program();
    ~Program();

    std::vector<Task *> tasks;
    uint8_t activeTaskId;
    Task *activeTask;

    friend Program *newProgram();

  public:
    void setup();
    void loop();
    Task *newTask(uint8_t id, bool hasOneLife = false);

    Task *getTaskById(uint8_t id);
  };

  Program *newProgram();

}

#endif // PROGRAM_H