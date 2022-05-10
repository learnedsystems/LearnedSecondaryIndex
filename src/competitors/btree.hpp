#pragma once

#include <tlx/container.hpp>

namespace lsi_competitors {
/**
 * Secondary index using Btree (updated STX version maintained as part of TLX
 * library)
 */
template <class Key, bool BulkLoad = false>
class BTree {
  tlx::btree_multimap<Key, size_t> _tree;

  using DisplacementVector = std::vector<std::pair<Key, size_t>>;

 public:
  BTree() noexcept = default;

  template <class It>
  BTree(const It &begin, const It &end) {
    fit(begin, end);
  };

  template <class It>
  void fit(const It &begin, const It &end) {
    if constexpr (BulkLoad) {
      const auto n = std::distance(begin, end);

      // retain original displacement for each key to build permutations vector
      DisplacementVector data;
      data.reserve(n);
      size_t i = 0;
      for (auto it = begin; it < end; it++, i++) {
        data.push_back(std::make_pair(*it, i));
      }

      // sort data
      std::sort(data.begin(), data.end(), [](const auto &d1, const auto &d2) {
        return d1.first < d2.first;
      });

      // bulk load data
      _tree.bulk_load(data.begin(), data.end());
    } else {
      size_t i = 0;
      for (auto it = begin; it < end; it++) _tree.insert2(*it, i++);
    }
  }

  /// Secondary Index Iterator
  class DisplacementIter {
    using BaseIter = typename decltype(_tree)::const_iterator;
    BaseIter _iter;

    explicit DisplacementIter(BaseIter iter) : _iter(iter) {}

   public:
    using iterator_category = typename BaseIter::iterator_category;
    using difference_type = typename BaseIter::difference_type;
    using value_type = size_t;
    using pointer = value_type *;
    using reference = value_type &;

    /// Obtain current offset into original data [begin, end)
    value_type operator*() const { return (*_iter).second; }

    /// Prefix increment
    DisplacementIter &operator++() {
      _iter++;
      return *this;
    }

    /// Postfix increment
    DisplacementIter operator++(int) {
      DisplacementIter tmp = *this;
      ++(*this);
      return tmp;
    }

    template <class T>
    DisplacementIter &operator+=(const T &o) {
      _iter += o;
      return *this;
    }

    template <class I>
    DisplacementIter operator+(const I &other) const {
      return DisplacementIter(_iter + other);
    }

    friend difference_type operator-(const DisplacementIter &a,
                                     const DisplacementIter &b) {
      return std::distance(a._iter, b._iter);
    }

    friend bool operator==(const DisplacementIter &a,
                           const DisplacementIter &b) {
      return a._iter == b._iter;
    };

    friend bool operator!=(const DisplacementIter &a,
                           const DisplacementIter &b) {
      return a._iter != b._iter;
    };

    friend BTree<Key, BulkLoad>;
  };

  DisplacementIter begin() const { return DisplacementIter(_tree.begin()); }

  DisplacementIter end() const { return DisplacementIter(_tree.end()); }

  template <bool lowerbound, class It>
  DisplacementIter lookup(const It & /*begin*/, const It & /*end*/,
                          const Key &key) const {
    const auto it = _tree.lower_bound(key);
    return DisplacementIter(it);
  }

  size_t base_data_accesses() const { return 0; }

  size_t false_positive_accesses() const { return 0; }

  size_t model_byte_size() const {
    const auto stats = _tree.get_stats();
    return stats.inner_nodes * stats.inner_slots * (sizeof(Key)) +
           stats.inner_nodes * (stats.inner_slots + 1) * sizeof(void *);
  }

  size_t perm_vector_byte_size() const {
    const auto stats = _tree.get_stats();
    return stats.leaves * stats.leaf_slots * (sizeof(std::pair<Key, size_t>)) +
           2 * sizeof(void *);
  }

  size_t byte_size() const {
    return model_byte_size() + perm_vector_byte_size();
  }

  static std::string name() { return "BTree"; }
};
}  // namespace lsi_competitors
