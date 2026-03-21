#pragma once
#include <closure.hpp>
#include <scheduler.hpp>

class Runtime {
public:
  static Runtime &getInstance();

  void spawn(Closure *);
  void yield();
  void run();

private:
  Scheduler scheduler;
};