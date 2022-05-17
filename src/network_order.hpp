#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef CAF_MSVC // we assume Windows is always little endian

inline uint16_t to_network_order(uint16_t value) {
  return _byteswap_ushort(value);
}

inline uint32_t to_network_order(uint32_t value) {
  return _byteswap_ulong(value);
}

inline uint64_t to_network_order(uint64_t value) {
  return _byteswap_uint64(value);
}

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

inline uint16_t to_network_order(uint16_t value) {
  return __builtin_bswap16(value);
}

inline uint32_t to_network_order(uint32_t value) {
  return __builtin_bswap32(value);
}

inline uint64_t to_network_order(uint64_t value) {
  return __builtin_bswap64(value);
}

#else

template <class T> T to_network_order(T value) { return value; }

#endif

template <class T> T from_network_order(T value) {
  // swapping the bytes again gives the native order
  return to_network_order(value);
}
