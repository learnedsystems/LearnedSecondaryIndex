#pragma once

#include <cstdint>
#include <iterator>
#include <limits>

#include "../convenience/builtins.hpp"
#include "bitpacking/bit_packing.h"
#include "fingerprinter.hpp"
#include "support.hpp"

namespace learned_secondary_index::util {
/// Packed vector containing permutation information
template <class F>
class PermVector {
  size_t _size;
  std::string _data;
  ci::BitPackedReader<uint64_t> _offsets_reader;
  ci::BitPackedReader<uint64_t> _fingerprint_bits_reader;

  // generate fingerprint bits using this
  F _fingerprinter;

  struct Value {
    const uint64_t index = 0;
    const uint64_t fingerprint_bits = 0;
  };

  Value deserialize(const size_t &i) const {
    const size_t index = _offsets_reader.Get(i);

    if constexpr (F::size == 0) {
      return {.index = index, .fingerprint_bits = 0};
    } else {
      uint64_t fingerprint_bits = _fingerprint_bits_reader.Get(i);
      return {.index = index, .fingerprint_bits = fingerprint_bits};
    }
  }

 public:
  using value = Value;

  /// Constructs an empty permutations vector with no data
  PermVector() : _size(0) {}

  class PermIterator {
    const PermVector &_ref;
    size_t _index;

    PermIterator(const PermVector &ref, const size_t &index)
        : _ref(ref), _index(index) {}

   public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = size_t;
    using value_type = Value;
    using pointer = value_type *;
    using reference = value_type &;

    /// Obtain current offset into original data [begin, end)
    value_type operator*() const { return _ref.deserialize(_index); }

    // Prefix increment
    PermIterator &operator++() {
      _index++;
      return *this;
    }

    // Postfix increment
    PermIterator operator++(int) {
      PermIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    // Assign add
    template <class T>
    PermIterator &operator+=(const T &o) {
      _index += o;
      return *this;
    }

    template <class I>
    PermIterator operator+(const I &other) const {
      return PermIterator(_ref, _index + other);
    }

    template <class I>
    PermIterator operator-(const I &other) const {
      return PermIterator(_ref, _index - other);
    }

    friend difference_type operator-(const PermIterator &a,
                                     const PermIterator &b) {
      return a._index - b._index;
    }

    friend bool operator<(const PermIterator &a, const PermIterator &b) {
      return a._index < b._index;
    }

    friend bool operator==(const PermIterator &a, const PermIterator &b) {
      return a._index == b._index && &a._ref == &b._ref;
    };

    friend bool operator!=(const PermIterator &a, const PermIterator &b) {
      return a._index != b._index || &a._ref != &b._ref;
    };

    friend PermVector<F>;
  };

  using iterator = PermIterator;

  /**
   * Builds permutations vector on elements in [begin, end), returning Bloomer
   * used for generating associated fingerprint bits.
   */
  template <class ForwardIt>
  void build(const ForwardIt &begin, const ForwardIt &end) {
    _size = std::distance(begin, end);

    std::vector<uint64_t> offsets;
    std::vector<uint64_t> fingerprint_bits;

    for (auto it = begin; it < end; it++) {
      offsets.push_back(*it);

      if constexpr (F::size > 0) {
        fingerprint_bits.push_back(_fingerprinter.fingerprint(it.key()));
      }
    }

    ci::ByteBuffer result;

    const int offsets_bit_width = ci::MaxBitWidth<uint64_t>(offsets);
    ci::StoreBitPacked<uint64_t>(offsets, offsets_bit_width, &result);

    int fingerprint_bits_bit_width;
    size_t fingerprint_bits_pos;
    if constexpr (F::size > 0) {
      fingerprint_bits_bit_width = ci::MaxBitWidth<uint64_t>(fingerprint_bits);
      fingerprint_bits_pos = result.pos();
      ci::StoreBitPacked<uint64_t>(fingerprint_bits, fingerprint_bits_bit_width,
                                   &result);
    }

    PutSlopBytes(&result);

    _data = std::string(result.data(), result.pos());
    _offsets_reader =
        ci::BitPackedReader<uint64_t>(offsets_bit_width, _data.data());

    if constexpr (F::size > 0) {
      _fingerprint_bits_reader = ci::BitPackedReader<uint64_t>(
          fingerprint_bits_bit_width, _data.data() + fingerprint_bits_pos);
    }
  }

  /// Index based access into permutations vector
  Value operator[](const size_t &index) const {
    return *PermIterator(*this, index);
  }

  /// Tests whether a key k matches the fingerprint bits stored in value v
  template <class K>
  bool test(const K &k, const Value &v) const {
    return _fingerprinter.test(k, v.fingerprint_bits);
  }

  /// Iterator to first entry in PermVector
  PermIterator begin() const { return PermIterator(*this, 0); }

  /// Past the end Iterator for PermVector
  PermIterator end() const { return PermIterator(*this, _size); }

  /// Element count of PermVector
  [[nodiscard]] decltype(_size) size() const { return _size; }

  /// Total memory occupied by this PermVector in bytes
  [[nodiscard]] size_t byte_size() const {
    return sizeof(PermVector) + _data.size();
  }

  friend bool operator==(const PermVector &a, const PermVector &b) {
    return a._data == b._data && a._size == b._size;
  }

  friend bool operator!=(const PermVector &a, const PermVector &b) {
    return !(a == b);  // NOLINT
  }
};
}  // namespace learned_secondary_index::util
