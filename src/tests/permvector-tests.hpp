#pragma once

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <learned_secondary_index.hpp>
#include <limits>
#include <random>

#include "include/util/permvector.hpp"

void test_permvector_width(const size_t width) {
  using Key = std::uint64_t;

  using ::learned_secondary_index::LearnedSecondaryIndex;
  using ::learned_secondary_index::util::Fingerprinter;
  using ::learned_secondary_index::util::PermVector;

  std::random_device rd;
  std::default_random_engine rng(rd());
  std::uniform_int_distribution<Key> dist(0, 1LLU << (width - 1));

  for (const auto size : {0UL, 10UL, 1000UL, 100000UL}) {
    // gen permutation vector
    std::vector<std::pair<Key, size_t>> dataset;
    dataset.reserve(size);
    for (size_t i = 0; i < size; i++) {
      dataset.emplace_back(dist(rng), i);
    }

    const LearnedSecondaryIndex<Key>::PairIter<true> pb(dataset.begin());
    const LearnedSecondaryIndex<Key>::PairIter<true> pe(dataset.end());

    // build permutation vector
    PermVector<Fingerprinter<Key, 8>> pv;
    pv.build(pb, pe);

    EXPECT_EQ(pv.size(), dataset.size());

    // test index access
    for (auto i = 0UL; i < pv.size(); i++) {
      EXPECT_EQ(pv[i].index, dataset[i].first);
    }

    // test iterator access
    for (auto it = pv.begin(); it < pv.end(); it++) {
      EXPECT_EQ((*it).index, dataset[std::distance(pv.begin(), it)].first);
    }
  }
}

TEST(PermVector, E2E) {
  for (size_t i = 1; i <= 64; ++i) {
    test_permvector_width(i);
  }
}
