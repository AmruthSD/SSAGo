#include <scheduler.hpp>

void Scheduler::enqueue(Fiber *f) { run_queue.push_back(f); }

void Scheduler::run() {
  while (!run_queue.empty()) {
    current = run_queue.front();
    run_queue.pop_front();

    current->resume();

    if (!current->isFinished()) {
      run_queue.push_back(current);
    } else {
      delete current;
    }
  }
}

void Scheduler::yield_current(boost::context::fiber &&ctx) {
  main_ctx = std::move(ctx);
}