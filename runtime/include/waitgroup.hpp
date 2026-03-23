#pragma once
#include <atomic>
#include <deque>

class Fiber;
class Scheduler;

class WaitGroup {
public:
  WaitGroup() : count(0) {}

  void add(int n);
  void done();
  void wait();

private:
  std::atomic<int> count;
  std::deque<Fiber *> waiting;
};