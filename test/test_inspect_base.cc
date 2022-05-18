#include <iostream>
#include "../src/save_inspector_base.hpp"

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
  save_inspector_base<inspec> sib;
  

  return 0;
}