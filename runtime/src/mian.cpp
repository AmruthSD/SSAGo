#include <external.hpp>
#include <iostream>

extern "C" int __user_main();

extern "C" void __user_main_wrapper(void *args) { __user_main(); }

int main() {
  std::cout << "hello runtime"
            << "\n";

  runtime_spawn((void *)__user_main_wrapper, nullptr);

  Runtime::getInstance().run();
}