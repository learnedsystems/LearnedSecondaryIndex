#pragma once

#include <cstddef>
#include <cstdint>
#include <x86intrin.h>

#include "../convenience/builtins.hpp"

namespace learned_secondary_index::util {
/// Lower bound implementation from
/// https://en.cppreference.com/w/cpp/algorithm/lower_bound, adapted to be
/// usable on std::vector as well as support::EliasFanoList
template <class Dataset, class Data>
forceinline size_t lower_bound(size_t first, size_t last, const Data &value,
                               const Dataset &dataset) {
  size_t i = first, count = last - first, step = 0;

  while (count > 0) {
    i = first;
    step = count / 2;
    i += step;
    if (dataset[i] < value) {
      first = ++i;
      count -= step + 1;
    } else
      count = step;
  }
  return first;
};

template <class T> forceinline constexpr size_t ffs(const T &x) {
  switch (sizeof(T)) {
  case sizeof(std::uint64_t):
    return __builtin_ffsll(x);
  case sizeof(std::uint32_t):
    return __builtin_ffs(x);
  default:
    size_t i = 0;
    while (~((x >> i) & 0x1))
      i++;
    return i;
  }
}

template <class T> forceinline constexpr size_t ctz(const T &x) {
  switch (sizeof(T)) {
  case sizeof(std::uint32_t):
    return __tzcnt_u32(x);
  case sizeof(std::uint64_t):
    return __tzcnt_u64(x);
  default:
    size_t i = 0;
    while (~((x >> i) & 0x1))
      i++;
    return i;
  }
}

template <class T> forceinline constexpr size_t clz(const T &x) {
  if (x == 0)
    return sizeof(T) * 8;

  switch (sizeof(T)) {
  case sizeof(std::uint32_t):
    return __builtin_clz(x);
  case sizeof(std::uint64_t):
    return __builtin_clzll(x);
  default:
    size_t i = 0;
    while (((x >> (sizeof(T) * 8 - i - 1)) & 0x1) == 0x0)
      i++;
    return i;
  }
}

template <class T> forceinline constexpr T bitreverse(const T &x) {
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

  constexpr const auto bitcnt = sizeof(T) * 8;

  switch (bitcnt) {
#if __has_builtin(__builtin_bitreverse64)
  case 64:
    return __builtin_bitreverse64(x);
#endif
#if __has_builtin(__builtin_bitreverse32)
  case 32:
    return __builtin_bitreverse32(x);
#endif
#if __has_builtin(__builtin_bitreverse16)
  case 16:
    return __builtin_bitreverse16(x);
#endif
#if __has_builtin(__builtin_bitreverse8)
  case 8:
    return __builtin_bitreverse8(x);
#endif
  default: {
    // default to unoptimized implementation
    T r_elem = 0;
    for (size_t i = 0; i < bitcnt; i++) {
      r_elem <<= 1;
      r_elem |= (x >> i) & 0x1;
    }
    return r_elem;
  }
  }
}
} // namespace learned_secondary_index::util
