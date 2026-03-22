#pragma once

#include <boost/context/fiber.hpp>
#include <closure.hpp>

class Fiber {
public:
  using Context = boost::context::fiber;

private:
  Context ctx;
  Context sched_ctx;
  Closure *closure;
  bool finished = false;

public:
  Fiber(Closure *c);

  void resume();
  void yield();

  bool isFinished() const { return finished; }
};