#pragma once

#include <string>
#include <vector>

#include "fast64/fast64.h"

namespace lsi_competitors {

template <class Key>
class FAST64 {
  static_assert(sizeof(Key) == 64 / 8,
                "This implementation of FAST does only support 64 bit keys.");

 private:
  class Iterator;

 public:
  FAST64() noexcept = default;

  template <class It>
  FAST64(const It& begin, const It& end) {
    fit(begin, end);
  }

  ~FAST64() {
    if (tree_) {
      destroy_fast64(tree_);
    }
  }

  template <class It>
  void fit(const It& begin, const It& end) {
    std::vector<Key> keys(begin, end);
    data_size_ = keys.size();

    std::vector<std::uint64_t> values;
    values.reserve(data_size_);
    for (size_t i = 0; i < data_size_; ++i) {
      values.push_back(i);
    }

    std::sort(values.begin(), values.end(),
              [&keys](const size_t s1, const size_t s2) {
                return keys[s1] < keys[s2];
              });
    std::sort(keys.begin(), keys.end());

    tree_ =
        create_fast64(keys.data(), keys.size(), values.data(), values.size());
  }

  template <bool lowerbound, class It>
  Iterator lookup(const It& /*begin*/, const It& /*end*/, const Key& key) {
    std::uint64_t lb, ub;
    lookup_fast64(tree_, key, &lb, &ub);
    ub = std::min(data_size_, ub);
    return Iterator(lb, ub);
  }

  Iterator begin() const { return Iterator(0, data_size_); }

  Iterator end() const { return Iterator(data_size_, data_size_); }

  static std::string name() { return "FAST64"; }

  size_t base_data_accesses() const { return 0; }

  size_t false_positive_accesses() const { return 0; }

  [[nodiscard]] std::size_t byte_size() const {
    return perm_vector_byte_size() + model_byte_size();
  }

  [[nodiscard]] std::size_t perm_vector_byte_size() const { return 0; }

  [[nodiscard]] std::size_t model_byte_size() const {
    return size_fast64(tree_);
  }

 private:
  uint64_t data_size_ = 0;
  Fast64* tree_ = nullptr;

  class Iterator {
   public:
    Iterator(const size_t lb, const size_t ub) noexcept
        : current(lb), upper_bound(ub){};

    uint64_t operator*() const { return current; }

    Iterator& operator++(int) {
      if (current < upper_bound) {
        current++;
      }
      return *this;
    }

    friend bool operator==(const Iterator& a, const Iterator& b) {
      return a.current == b.current && a.upper_bound == b.upper_bound;
    }

    friend bool operator!=(const Iterator& a, const Iterator& b) {
      return !(a == b);  // NOLINT Simplify
    }

   private:
    size_t current;
    const size_t upper_bound;
  };
};

}  // namespace lsi_competitors
