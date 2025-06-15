/**
 * @file src/Program.cpp
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

#include "Program.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>

namespace MABQ
{
  void Program::Task::transitionTo(uint8_t id)
  {
    if (!parentProgram)
    {
      if (MABQ::gDebug && MABQ::gDebug->enabled)
        gDebug->log("Parent program not set in Task.");
      throw std::runtime_error("Parent program not set in Task.");
    }

    this->exiting = true;

    Task *targetTask = parentProgram->getTaskById(id);
    if (!targetTask)
    {
      if (MABQ::gDebug && MABQ::gDebug->enabled)
        gDebug->log("Task with requested id not found.");
      throw std::runtime_error("Task with requested id not found.");
    }

    parentProgram->activeTaskId = id;

    targetTask->settingUp = true;
  }

  Program::Task::Task(uint8_t id, bool hasOneLife, Program *parent)
      : id(id), oneLife(hasOneLife), settingUp(false), 
        loopCount(0), exiting(false),
        onSetup([](Task *) {}),
        parentProgram(parent)
  {
    onLoop = [this](Task *task)
    {
      std::ostringstream oss;
      oss << "Task ID " << static_cast<int>(this->id)
          << ": No loop function provided. Please set onLoop or switch to another task in setup().";

      if (MABQ::gDebug && MABQ::gDebug->enabled)
        MABQ::gDebug->log(oss.str().c_str());
      throw std::runtime_error(oss.str());
    };
  }


  void Program::Task::setup()
  {
    onSetup(this);
    settingUp = false;
  }

  void Program::Task::loop()
  {
    onLoop(this);
    loopCount++;
  }

  void Program::Task::exit()
  {
    exiting = false;
  }

  Program::Program()
    : activeTaskId(0), activeTask(nullptr)
  {
  }

  Program::~Program()
  {
    for (auto task : tasks)
    {
      delete task;
    }
    tasks.clear();
  }

  static auto defaultOnLoop = [](Program::Task *task)
  {
    std::ostringstream oss;
    oss << "Task ID " << static_cast<int>(task->id)
        << ": No loop function provided. Please set onLoop or switch to another task in setup().";

    if (MABQ::gDebug && MABQ::gDebug->enabled)
      MABQ::gDebug->log(oss.str().c_str());
    else
      throw std::runtime_error(oss.str());
  };

  void Program::setup()
  {
    if (tasks.empty())
      throw std::runtime_error("No tasks available to setup.");

    activeTask = getTaskById(activeTaskId);
    activeTask->setup();
  }

  void Program::loop()
  {
    if (!activeTask)
      return;

    if (activeTask->settingUp)
      activeTask->setup();

    if (!activeTask->exiting)
      activeTask->loop();

    if (activeTask->exiting)
    {
      if (activeTask->oneLife)
      {
        auto it = std::find(tasks.begin(), tasks.end(), activeTask);
        if (it != tasks.end())
          tasks.erase(it);

        delete activeTask;
        activeTask = nullptr;
      }

      activeTask = getTaskById(activeTaskId);
    }
  }

  Program::Task *Program::newTask(uint8_t id, bool hasOneLife)
  {
    Task *task = new Task(id, hasOneLife, this);
    tasks.push_back(task);
    return task;
  }

  Program::Task *Program::getTaskById(uint8_t id)
  {
    auto it = std::find_if(
        tasks.begin(), tasks.end(), [id](Task *task)
        { return task->id == id; });

    if (it != tasks.end())
      return *it;
    return nullptr;
  }

  Program *newProgram()
  {
    return new Program();
  }

}
