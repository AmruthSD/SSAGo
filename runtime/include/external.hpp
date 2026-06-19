#pragma once
#include <closure.hpp>
#include <runtime.hpp>
#include <waitgroup.hpp>

extern "C" void runtime_spawn(void *fn, void *args) {
  using FnType = void (*)(void *);
  FnType f = (FnType)fn;

  Closure *c = new Closure(f, args);

  Runtime::getInstance().spawn(c);
}

extern "C" void yield() {
  std::cout << "yeild  made" << std::endl;
  Runtime::getInstance().yield();
}

extern "C" {
void *waitgroup_new() { return new WaitGroup(); }

void waitgroup_add(void *wg, int n) { ((WaitGroup *)wg)->add(n); }

void waitgroup_done(void *wg) { ((WaitGroup *)wg)->done(); }

void waitgroup_wait(void *wg) { ((WaitGroup *)wg)->wait(); }

void waitgroup_delete(void *wg) { delete (WaitGroup *)wg; }
}