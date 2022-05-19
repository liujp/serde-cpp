#include "../src/binary_serializer.hpp"
#include <bitset>
#include <cstddef>
#include <iostream>
std::ostream &operator<<(std::ostream &os, std::byte b) {
  return os << std::bitset<8>(std::to_integer<int>(b));
}
int main() {
  std::vector<std::byte> si;
  si.reserve(100);
  binary_serializer bs(si);
  auto disp = [](const binary_serializer &b) {
    for (auto &i : b.buf()) {
      std::cout << i;
    }
    std::cout << std::endl;
  };
  bs.value(true);
  disp(bs);
  bs.value((uint8_t)10);
  disp(bs);
  bs.value((int8_t)10);
  disp(bs);
  bs.value((int16_t)20);
  disp(bs);
  bs.value((uint16_t)20);
  disp(bs);
  bs.value((int32_t)30);
  disp(bs);
  bs.value((uint32_t)30);
  disp(bs);
  bs.value(std::string("abcd"));
  disp(bs);
}