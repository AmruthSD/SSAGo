#include <runtime.hpp>
#include <scheduler.hpp>
#include <waitgroup.hpp>

void WaitGroup::add(int n) { count.fetch_add(n); }

void WaitGroup::done() {
  int prev = count.fetch_sub(1);
  if (prev == 1) {
    while (!waiting.empty()) {
      Fiber *f = waiting.front();
      waiting.pop_front();
      f->suspended = false;
      Runtime::getInstance().scheduler.enqueue(f);
    }
  }
}

void WaitGroup::wait() {
  while (count.load() > 0) {
    Fiber *self = Runtime::getInstance().scheduler.current;
    self->suspended = true;
    waiting.push_back(self);
    self->yield();
  }
}