#pragma once

#include <deque>
#include <fiber.hpp>

class Scheduler {
private:
  std::deque<Fiber *> run_queue;
  boost::context::fiber main_ctx;

public:
  void enqueue(Fiber *f);

  void run();

  void yield_current();

private:
  Fiber *current = nullptr;
};