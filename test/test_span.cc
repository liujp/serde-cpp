#include "../src/binary_serializer.hpp"
#include "../src/my_error.hpp"
#include "../src/span.hpp"
#include <iostream>
#include <type_traits>
int main() {
  int a[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  span<int> sp(a, 10);
  std::cout << *sp.begin() << std::endl;
  std::cout << std::boolalpha << std::is_same<int, error>::value << std::endl;
  std::cout << std::boolalpha << std::is_same<error, error>::value << std::endl;
  byte_buffer bf;
  bf.resize(10);
  binary_serializer ser(bf);
  std::cout << "binary serialize: " << ser.buf().size() << "\n";
  return 0;
}