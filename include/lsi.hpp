#pragma once

#include <algorithm>
#include <cstdint>
#include <learned_hashing.hpp>
#include <type_traits>
#include <utility>
#include <vector>

#include "convenience/builtins.hpp"
#include "include/rs.hpp"
#include "include/util/fingerprinter.hpp"
#include "util/permvector.hpp"

namespace learned_secondary_index {

/**
 * Secondary Index implementation utilizing a learned CDF
 * tuned to the data at hand.
 *
 * @tparam Key key type, e.g., std::uint64_t for most SOSD datasets
 * @tparam Model CDF Model
 * @tparam Displacement data type used for internal displacement/permutations
 * vector. Choose large enough to fit data.size()! Should become irrelevant in
 * the future due to bitpacking etc.
 */
template <class Key,
          class Model = learned_hashing::RadixSplineHash<Key, 18, 16>,
          std::uint8_t fingerprint_size = 0, bool force_linear_search = false>
class LearnedSecondaryIndex {
  util::PermVector<util::Fingerprinter<Key, fingerprint_size>> _perm_vector;
  Model _model;
  size_t max_error = 0;

  using DisplacementVector = std::vector<std::pair<Key, size_t>>;

  size_t _base_data_accesses = 0;
  size_t _false_positive_accesses = 0;

 public:
  /// Speeds up permutation and model build
  template <bool first = true>
  class PairIter {
    using BaseIter = typename DisplacementVector::iterator;

    BaseIter _iter;
    const size_t _skip;

    /// Returns _skip if _skip != 0, 1 otherwise
    forceinline constexpr size_t gap() const { return _skip + (_skip == 0); }

   public:
    using iterator_category = typename BaseIter::iterator_category;
    using difference_type = typename BaseIter::difference_type;
    using value_type = typename std::conditional<first, Key, size_t>::type;
    using pointer = value_type *;
    using reference = value_type &;

    explicit PairIter(BaseIter iter, size_t skip = 0)
        : _iter(iter), _skip(skip) {}

    [[nodiscard]] typename BaseIter::value_type::first_type &key() const {
      return _iter->first;
    }

    [[nodiscard]] typename BaseIter::value_type::second_type &displacement()
        const {
      return _iter->second;
    }

    reference operator*() const {
      if constexpr (first) {
        return _iter->first;
      } else {
        return _iter->second;
      }
    }

    pointer operator->() {
      if constexpr (first) {
        return _iter->first;
      } else {
        return _iter->second;
      }
    }

    // Prefix increment
    PairIter &operator++() {
      _iter += gap();
      return *this;
    }

    // Postfix increment
    PairIter operator++(int) {
      PairIter tmp = *this;
      ++(*this);
      return tmp;
    }

    template <class I>
    PairIter operator+(const I &other) const {
      return PairIter(_iter + other * gap(), _skip);
    }

    template <class I>
    PairIter operator-(const I &other) const {
      return PairIter(_iter - other * gap(), _skip);
    }

    friend difference_type operator-(const PairIter &a, const PairIter &b) {
      return (a._iter - b._iter) / a.gap();
    }

    friend bool operator<(const PairIter &a, const PairIter &b) {
      return a._iter < b._iter;
    }

    friend bool operator==(const PairIter &a, const PairIter &b) {
      return a._iter == b._iter;
    };

    friend bool operator!=(const PairIter &a, const PairIter &b) {
      return a._iter != b._iter;
    };

    friend LearnedSecondaryIndex<Key, Model, fingerprint_size,
                                 force_linear_search>;
  };

 public:
  /// Constructs an empty index
  LearnedSecondaryIndex() noexcept = default;

  template <class It>
  LearnedSecondaryIndex(const It &begin, const It &end) {
    fit(begin, end);
  }

  /**
   * Builds displacement/permutations vector and fits cdf model to concrete
   * datapoints. Since [begin, end) is most likely unsorted, this function
   * will use an additional O(n) storage to build a temporary, sorted
   * representation for the model training step.
   *
   * @param begin start of data to index
   * @param end start of data to index
   */
  template <class It>
  void fit(const It &begin, const It &end) {
    const auto n = std::distance(begin, end);

    // retain original displacement for each key to build permutations vector
    DisplacementVector data;
    data.reserve(n);
    size_t i = 0;
    for (auto it = begin; it < end; it++) {
      data.push_back(std::make_pair(*it, i++));
    }

    // sort data
    std::sort(data.begin(), data.end(), [](const auto &d1, const auto &d2) {
      return d1.first < d2.first;
    });

    // build permutations vector
    const PairIter<false> pb(data.begin());
    const PairIter<false> pe(data.end());
    assert(std::distance(pb, pe) == n);
    _perm_vector.build(pb, pe);

    // build learned model
    // TODO(dominik): don't build on full data by utilizing available skip
    // property (?)
    const PairIter<true> db(data.begin());
    const PairIter<true> de(data.end());
    assert(std::distance(db, de) == n);
    _model.train(db, de, data.size());

    // retain model's max error on this data
    // TODO(dominik): speed up by determining this during training
    max_error = 0;
    size_t current_lower_bound = 0;
    for (size_t j = 0; j < data.size(); j++) {
      if (data[current_lower_bound].first != data[j].first) {
        current_lower_bound = j;
      }

      const auto pred = _model(data[j].first);
      const decltype(max_error) err = std::max(pred, current_lower_bound) -
                                      std::min(pred, current_lower_bound);

      max_error = std::max(max_error, err);
    }
  }

  /**
   * Iterator on the internal displacement/permutations vector.
   * Dereferencing yields an offset into the original, unsorted data range
   * [begin, end)
   */
  class PermIter {
    using BaseIter = typename decltype(_perm_vector)::iterator;
    using Vector = decltype(_perm_vector);

    size_t _index;
    const Vector &_perm_vector;

    PermIter(size_t index, decltype(_perm_vector) &perm_vector)
        : _index(index), _perm_vector(perm_vector) {}

    typename Vector::value value() const { return _perm_vector[_index]; }

   public:
    using iterator_category = typename BaseIter::iterator_category;
    using difference_type = typename BaseIter::difference_type;
    using value_type = size_t;
    using pointer = value_type *;
    using reference = value_type &;

    /// Obtain current offset into original data [begin, end)
    value_type operator*() const { return _perm_vector[_index].index; }

    // Prefix increment
    PermIter &operator++() {
      _index++;
      return *this;
    }

    // Postfix increment
    PermIter operator++(int) {
      PermIter tmp = *this;
      ++(*this);
      return tmp;
    }

    // Assign add
    template <class T>
    PermIter &operator+=(const T &o) {
      _index += o;
      return *this;
    }

    template <class I>
    PermIter operator+(const I &other) const {
      return PermIter(_index + other, _perm_vector);
    }

    template <class I>
    PermIter operator-(const I &other) const {
      return PermIter(_index - other, _perm_vector);
    }

    friend difference_type operator-(const PermIter &a, const PermIter &b) {
      return a._index - b._index;
    }

    friend bool operator<(const PermIter &a, const PermIter &b) {
      return a._index < b._index;
    }

    friend bool operator>=(const PermIter &a, const PermIter &b) {
      return !(a < b);  // NOLINT
    }

    friend bool operator==(const PermIter &a, const PermIter &b) {
      return a._index == b._index && a._perm_vector == b._perm_vector;
    };

    friend bool operator!=(const PermIter &a, const PermIter &b) {
      return a._index != b._index || a._perm_vector != b._perm_vector;
    };

    friend LearnedSecondaryIndex<Key, Model, fingerprint_size,
                                 force_linear_search>;
  };

  /// PermIter pointing to the first stored displacement
  PermIter begin() const { return PermIter(0, _perm_vector); }

  /// Past-the-end iterator
  PermIter end() const { return PermIter(_perm_vector.size(), _perm_vector); }

  /**
   * Lookup key in range [begin, end). Note that [begin, end) must
   * point to a range of the same size and ordering as provided to the previous
   * fit() call.
   *
   * @tparam lowerbound whether to perform a lowerbound or equality lookup
   * @param begin start of relation range
   * @param end past-the-end of relation range
   * @param key to search
   *
   * @returns if lowerbound=false iterator that yields the offset into [begin,
   * end) at which key may be found or end() if there is no such key. If
   * lowerbound=true, iterator that yields the offset into [begin, end) at which
   * the first entry not less than key may be found or end() if all keys are
   * smaller
   */
  template <bool lowerbound, class It>
  PermIter lookup(const It &begin, const It &end, const Key &key) const {
    const auto incr_false_positives = [&]() {
      // false_positive_accesses is a debug variable, only attached to LSI for
      // convenience/to not break the interface (const). Don't do this
      // for production code :)
      ((typename std::remove_const<  // NOLINT google-readability-casting
           typename std::remove_pointer<decltype(this)>::type>::type *)this)
          ->_false_positive_accesses++;
    };

    const auto incr_base_accesses = [&]() {
      // base_data_accesses is a debug variable, only attached to LSI for
      // convenience/to not break the interface (const). Don't do this
      // for production code :)
      ((typename std::remove_const<  // NOLINT google-readability-casting
           typename std::remove_pointer<decltype(this)>::type>::type *)this)
          ->_base_data_accesses++;
    };

    // predict rough displacement location
    const auto pred = _model(key);

    // compute start iter of search interval
    auto start_i =
        pred - std::min(pred, static_cast<decltype(pred)>(max_error));

    // function global constants
    auto stop_i = std::min(pred + max_error + 1, _perm_vector.size());
    const auto stop = this->begin() + stop_i;

    if constexpr (force_linear_search || fingerprint_size > 0) {
      PermIter ind(start_i, _perm_vector);

      // linear search algorithm
      for (; ind < stop; ind++) {
        const auto perm_val = ind.value();

        // use fingerprint bits to fast track non-hits.
        if constexpr (!lowerbound) {
          if (!_perm_vector.test(key, perm_val)) {
            assert(*(begin + perm_val.index) != key);
            continue;
          }
        }

        // access base data to see if we may stop
        incr_base_accesses();
        if (*(begin + perm_val.index) >= key) break;
        incr_false_positives();
      }

      if constexpr (lowerbound) {
        while (ind != this->end() && *(begin + *ind) < key) {
          incr_base_accesses();
          ind++;
        }
      } else {
        if (*(begin + *ind) != key) {
          return this->end();
        }
      }

      return ind;
    } else {
      // binary search
      while (start_i < stop_i) {
        // probe key at mid
        const auto mid_i = start_i + (stop_i - start_i) / 2;
        incr_base_accesses();
        const auto probed = *(begin + _perm_vector[mid_i].index);

        if (probed < key) {
          start_i = mid_i + 1;
        } else {
          stop_i = mid_i;
        }
      }

      PermIter ind(start_i, _perm_vector);

      if constexpr (lowerbound) {
        while (ind != this->end() && *(begin + *ind) < key) {
          incr_base_accesses();
          ind++;
        }
      } else {
        if (*(begin + *ind) != key) {
          return this->end();
        }
      }

      return ind;
    }
  }

  size_t base_data_accesses() const { return _base_data_accesses; }

  size_t false_positive_accesses() const { return _false_positive_accesses; }

  size_t model_byte_size() const { return _model.byte_size(); }

  size_t perm_vector_byte_size() const { return _perm_vector.byte_size(); }

  /// Computes total index size in bytes
  size_t byte_size() const {
    return sizeof(max_error) + model_byte_size() + perm_vector_byte_size();
  }

  static std::string name() {
    return "LSI<" + Model::name() + ", " + std::to_string(fingerprint_size) +
           ", " + std::to_string(force_linear_search) + ">";
  }
};
}  // namespace learned_secondary_index
