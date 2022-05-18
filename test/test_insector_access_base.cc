#include <iostream>
#include "../src/inspector_access_base.hpp"
#include <string>
#include <string_view>

struct inspec {
    bool begin_field(std::string_view x) {return true;}
    bool apply(int x) {return true;}
    bool emplace_error(int32_t x, const std::string & m){return true;}
    bool get_error(){return true;}
    bool end_field(){return true;}
};

typedef bool valid(int);
typedef bool sync();
bool is_valid(int x){return true;}
bool is_sync(){return true;}
int main() {
  inspec i;
  std::string_view xx = "ss";
  int t = 10;
  inspector_access_base<int> iab;
  inspector_access_base<int>::load_field<inspec, valid, sync>(i, xx, t, is_valid, is_sync);
  inspector_access_base<int>::save_field<inspec>(i, xx, t);
  return 0;
}