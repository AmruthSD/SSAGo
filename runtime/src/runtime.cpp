#include <runtime.hpp>

Runtime &Runtime::getInstance() {
  static Runtime instance;
  return instance;
}

void Runtime::spawn(Closure *c) {
  Fiber *f = new Fiber(c);
  scheduler.enqueue(f);
}

void Runtime::run() { scheduler.run(); }

void Runtime::yield() {}