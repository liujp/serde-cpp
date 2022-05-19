#include <bitset>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include "../src/binary_serializer.hpp"

std::ostream &operator<<(std::ostream &os, std::byte b) {
  return os << std::bitset<8>(std::to_integer<int>(b));
}

class Person {
public:
  std::string name;
  int age;
};

template <class Inspector> bool inspect(Inspector &f, Person &x) {
  return f.object(x).fields(f.field("name", x.name), f.field("age", x.age));
}

template <class... Ts> auto save(const Ts &... xs) {
  std::vector<std::byte> result;
  binary_serializer sink(result);
  if (!(sink.apply(xs) && ...))
    std::cerr << "binary_serializer failed to save: " << std::endl;
  else {
    for (const auto b : result) {
      std::cerr << b << ", ";
    }
    std::cerr << "\n";
  }
  return result;
}

template <class... Ts>
void save_to_buf(std::vector<std::byte> &data, const Ts &... xs) {
  binary_serializer sink(data);
  if (!(sink.apply(xs) && ...))
    std::cerr << "binary_serializer failed to save: " << std::endl;
  else {
    for (const auto b : sink.buf()) {
      std::cerr << b << ", ";
    }
    std::cerr << "\n";
  }
}

int main() {
  save(int8_t{10});
  save(int16_t{20});
  save(int32_t{30});
  save(int64_t{40});
  save(std::string{"hello"});
  save(std::vector<int>{1, 2, 3});
  std::cerr << "serialize person: "
            << "\n";
  Person p;
  p.name = "tom";
  p.age = 10;
  save(p);
  return 0;
}