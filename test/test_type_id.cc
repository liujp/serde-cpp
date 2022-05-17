#include "../src/type_id.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>

int main() {
  std::cout << type_id<bool>::value << "\n";
  std::cout << type_id<double>::value << "\n";
  std::cout << type_id<float>::value << "\n";
  std::cout << type_id<int16_t>::value << "\n";
  std::cout << type_id<int32_t>::value << "\n";
  std::cout << type_id<int64_t>::value << "\n";
  std::cout << type_id<int8_t>::value << "\n";
  std::cout << type_id<l_double>::value << "\n";
  std::cout << type_id<uint16_t>::value << "\n";
  std::cout << type_id<int32_t>::value << "\n";
  std::cout << type_id<uint64_t>::value << "\n";
  std::cout << type_id<uint8_t>::value << "\n";
  std::cout << type_id<stl_sting>::value << "\n";
  std::cout << type_id<stl_u16string>::value << "\n";
  std::cout << type_id<stl_u32string>::value << "\n";
  std::cout << type_id<stl_set_string>::value << "\n";

  std::cout << type_name<bool>::value << "\n";
  std::cout << type_name<double>::value << "\n";
  std::cout << type_name<float>::value << "\n";
  std::cout << type_name<int16_t>::value << "\n";
  std::cout << type_name<int32_t>::value << "\n";
  std::cout << type_name<int64_t>::value << "\n";
  std::cout << type_name<int8_t>::value << "\n";
  std::cout << type_name<l_double>::value << "\n";
  std::cout << type_name<uint16_t>::value << "\n";
  std::cout << type_name<int32_t>::value << "\n";
  std::cout << type_name<uint64_t>::value << "\n";
  std::cout << type_name<uint8_t>::value << "\n";
  std::cout << type_name<stl_sting>::value << "\n";
  std::cout << type_name<stl_u16string>::value << "\n";
  std::cout << type_name<stl_u32string>::value << "\n";
  std::cout << type_name<stl_set_string>::value << "\n";
  return 0;
}