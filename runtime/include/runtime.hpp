#pragma once
#include <atomic>
#include <closure.hpp>
#include <scheduler.hpp>

extern std::atomic<bool> should_yield;

class Runtime {
public:
  static Runtime &getInstance();

  void spawn(Closure *);
  void yield();
  void run();

  void init_scheduler();

private:
  Scheduler scheduler;
};