#pragma once

#include <emmintrin.h>
#include <mmintrin.h>

#include <cstdint>
#include <hashing.hpp>
#include <limits>
#include <random>
#include <unordered_set>

#include "../convenience/builtins.hpp"

namespace learned_secondary_index::util {
template <class Value, size_t fingerprint_size>
class Fingerprinter {
  static_assert(
      fingerprint_size < 64,
      "Due to implementation limitations a maximum of 64 fingerprint bits "
      "is supported");

  hashing::MurmurFinalizer<Value> _hash;

 public:
  static constexpr size_t size = fingerprint_size;

  std::uint64_t fingerprint(const Value &v) const {
    return _hash(v) & ((0x1LLU << fingerprint_size) - 1);
  }

  bool test(const Value &v, const std::uint64_t &print) const {
    return print == fingerprint(v);
  }
};
}  // namespace learned_secondary_index::util
