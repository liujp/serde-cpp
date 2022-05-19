#include "../src/inspector_access.hpp"
#include <iostream>

struct inspec {
  bool begin_field(std::string_view x) { return true; }
  bool apply(int x) { return true; }
  bool emplace_error(int32_t x, const std::string &m) { return true; }
  bool get_error() { return true; }
  bool end_field() { return true; }
};
typedef bool set(int);
typedef bool sync();
bool is_set(int x) { return true; }
bool is_sync() { return true; }

int main() {
  inspec i;
  int t = 10;
  bind_setter<inspec, set, int>(i, is_set, t);

  return 0;
}