#include <closure.hpp>
#include <fiber.hpp>

Fiber::Fiber(Closure *c) : closure(c) {
  ctx = Context([this](Context &&caller) {
    sched_ctx = std::move(caller);
    closure->execute();
    closure->destroy();
    finished = true;
    return std::move(sched_ctx);
  });
}

void Fiber::resume() { ctx = std::move(ctx).resume(); }

void Fiber::yield() { sched_ctx = std::move(sched_ctx).resume(); }