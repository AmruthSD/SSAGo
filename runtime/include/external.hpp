#pragma once
#include <closure.hpp>
#include <runtime.hpp>

extern "C" void runtime_spawn(void *fn, void *args) {
  using FnType = void (*)(void *);
  FnType f = (FnType)fn;

  Closure *c = new Closure(f, args);

  Runtime::getInstance().spawn(c);
}

extern "C" void yield() { Runtime::getInstance().yield(); }