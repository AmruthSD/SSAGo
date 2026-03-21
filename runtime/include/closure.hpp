#pragma once
#include <cstdlib>
#include <iostream>

class Closure {
public:
  using FnType = void (*)(void *);

private:
  FnType fn;
  void *args;

public:
  Closure(FnType fn, void *args) : fn(fn), args(args) {}

  void execute() { fn(args); }

  void destroy() { std::free(args); }

  FnType getFunction() const { return fn; }
  void *getArgs() const { return args; }
};