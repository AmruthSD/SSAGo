#include <iostream>

extern "C" int __user_main();

int main() {
  std::cout << "hello runtime"
            << "\n";

  int result = __user_main();
  return result;
}