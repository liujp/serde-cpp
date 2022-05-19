#include <bitset>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

#include "../src/binary_deserializer.hpp"
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

template <class... Ts>
void load(const std::vector<std::byte> &buf, Ts &... xs) {
  binary_deserializer source{nullptr, buf};
  if (!(source.apply(xs) && ...))
    std::cerr << "binary_deserializer failed to load: "
              << "\n";
}

template <class T> auto load(const std::vector<std::byte> &buf) {
  auto result = T{};
  load(buf, result);
  return result;
}

int main() {
  std::vector<std::byte> buffer;
  binary_serializer ser(buffer);
  Person p;
  p.name = "tom";
  p.age = 2;
  bool r = ser.apply(p);
  assert(r);
  for (const auto b : ser.buf()) {
    std::cerr << b << ", ";
  }
  std::cerr << "\n";

  binary_deserializer d_ser(buffer);
  Person t;
  r = d_ser.apply(t);
  assert(r);
  std::cout << "name: " << t.name << ", age: " << t.age << "\n";

  return 0;
}