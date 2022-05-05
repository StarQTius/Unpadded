#include <iostream>

extern uint64_t x;

template<auto &X>
auto f() {
  return sizeof X;
}

int main() {
  std::cout << "Size of `x`: " << f<x>() << std::endl;
  return 0;
}
