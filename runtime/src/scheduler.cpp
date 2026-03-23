#include <scheduler.hpp>

void Scheduler::enqueue(Fiber *f) { run_queue.push_back(f); }

void Scheduler::run() {
  while (!run_queue.empty()) {
    current = run_queue.front();
    run_queue.pop_front();

    current->resume();

    if (current->isFinished()) {
      delete current;
    } else if (!current->suspended) {
      run_queue.push_back(current);
    }
  }
}

void Scheduler::yield_current() { current->yield(); }