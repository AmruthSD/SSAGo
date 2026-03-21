#include <closure.hpp>
#include <fiber.hpp>

Fiber::Fiber(Closure *c) : closure(c) {
  ctx = Context([this](Context &&caller) {
    closure->execute();
    closure->destroy();
    finished = true;
    return std::move(caller);
  });
}

void Fiber::resume() {
  std::cout << "in fiber resume" << std::endl;
  ctx = std::move(ctx).resume();
}