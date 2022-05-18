#pragma once
#include <stdint.h>
#include <string>

// error = 0 success
enum error_code : int32_t {
  success = 0,
  unsafe_type,
  invalid_field_type,
  field_invariant_check_failed,
  field_value_synchronization_failed,
  save_callback_failed,
  error_num
};
using error = int32_t;
