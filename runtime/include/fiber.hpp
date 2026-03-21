#pragma once

#include <boost/context/fiber.hpp>
#include <closure.hpp>

class Fiber {
public:
  using Context = boost::context::fiber;

private:
  Context ctx;
  Closure *closure;
  bool finished = false;

public:
  Fiber(Closure *c);

  void resume();

  bool isFinished() const { return finished; }
};