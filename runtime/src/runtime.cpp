#include <runtime.hpp>
#include <signal.h>
#include <sys/time.h>

std::atomic<bool> should_yield{false};

Runtime &Runtime::getInstance() {
  static Runtime instance;
  return instance;
}

static void handle_alarm(int) { should_yield.store(true); }

void Runtime::init_scheduler() {
  struct sigaction sa;
  sa.sa_handler = handle_alarm;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGALRM, &sa, nullptr);

  struct itimerval timer;
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 10000;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 10000;
  setitimer(ITIMER_REAL, &timer, nullptr);
}

void Runtime::spawn(Closure *c) {
  Fiber *f = new Fiber(c);
  scheduler.enqueue(f);
}

void Runtime::run() { scheduler.run(); }

void Runtime::yield() {
  if (!should_yield.load())
    return;
  should_yield.store(false);
  scheduler.yield_current();
}