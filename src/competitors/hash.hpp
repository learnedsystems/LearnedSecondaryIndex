#pragma once

#include <tsl/robin_map.h>

namespace lsi_competitors {

template <class Key>
class RobinHash {
 private:
  class Iterator;  // forward declaration

 public:
  RobinHash() noexcept = default;

  template <class It>
  RobinHash(const It& begin, const It& end) {
    fit(begin, end);
  }

  template <class It>
  void fit(const It& begin, const It& end) {
    const auto data_size = std::distance(begin, end);

    map_.reserve(4 * data_size / 3);  // load factor 0.75

    size_t i = 0;
    for (auto it = begin; it < end; it++) {
      map_.insert({*it, i++});
    }
  }

  Iterator begin() const { return Iterator(map_.begin()); }

  Iterator end() const { return Iterator(map_.end()); }

  template <bool lowerbound, class It>
  Iterator lookup(const It& /*begin*/, const It& /*end*/,
                  const Key& key) const {
    static_assert(!lowerbound, "hash only supports equality lookups");
    return Iterator(map_.find(key));
  }

  size_t base_data_accesses() const { return 0; }

  size_t false_positive_accesses() const { return 0; }

  [[nodiscard]] std::size_t byte_size() const {
    return perm_vector_byte_size() + model_byte_size();
  }

  [[nodiscard]] std::size_t perm_vector_byte_size() const { return 0; }

  [[nodiscard]] std::size_t model_byte_size() const {
    return map_.bucket_count() * (sizeof(Key) + sizeof(uint64_t));
  }

  static std::string name() { return "RobinHash"; }

 private:
  tsl::robin_map<Key, uint64_t> map_;

  class Iterator {
    using BaseIter = typename decltype(map_)::const_iterator;

    const BaseIter _iter;

    explicit Iterator(BaseIter iter) : _iter(iter) {}

   public:
    using iterator_category = typename BaseIter::iterator_category;
    using difference_type = typename BaseIter::difference_type;
    using value_type = size_t;
    using pointer_type = value_type*;
    using reference_type = value_type&;

    /// Obtain current offset into original data [begin, end)
    value_type operator*() const { return (*_iter).second; }

    friend RobinHash<Key>;
  };
};

}  // namespace lsi_competitors
