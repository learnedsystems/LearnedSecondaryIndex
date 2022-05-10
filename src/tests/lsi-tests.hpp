#pragma once

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <learned_secondary_index.hpp>
#include <random>
#include <unordered_map>

using namespace learned_secondary_index;

namespace lsi_tests {

using Key = std::uint64_t;
using Model = learned_hashing::RadixSplineHash<Key, 18, 16>;

/// binary search test
TEST(LearnedSecondaryIndex, BinarySearch) {
  const auto datasize = 100000;

  // generate keys
  std::vector<Key> keys;
  keys.reserve(datasize);
  for (size_t i = 0; i < datasize; i++) keys.push_back(i + 20000);

  // shuffle keys (secondary index case)
  std::mt19937 rng(42);
  std::shuffle(keys.begin(), keys.end(), rng);

  // build LearnedSecondaryIndex on randomly shuffled keys
  LearnedSecondaryIndex<Key> lsi;
  lsi.fit(keys.begin(), keys.end());

  // test that retrieve works for all keys
  for (size_t i = 0; i < keys.size(); i++) {
    const auto iter = lsi.lookup<false>(keys.begin(), keys.end(), keys[i]);
    EXPECT_GE(iter, lsi.begin());
    EXPECT_LT(iter, lsi.end());
    EXPECT_EQ(*iter, i);
  }
}

/// linear search test
TEST(LearnedSecondaryIndex, LinearSearch) {
  const auto datasize = 100000;

  // generate keys
  std::vector<Key> keys;
  keys.reserve(datasize);
  for (size_t i = 0; i < datasize; i++) keys.push_back(i + 20000);

  // shuffle keys (secondary index case)
  std::mt19937 rng(42);
  std::shuffle(keys.begin(), keys.end(), rng);

  // build LearnedSecondaryIndex on randomly shuffled keys
  LearnedSecondaryIndex<Key, Model, 0, true> lsi;
  lsi.fit(keys.begin(), keys.end());

  // test that retrieve works for all keys
  for (size_t i = 0; i < keys.size(); i++) {
    const auto iter = lsi.lookup<false>(keys.begin(), keys.end(), keys[i]);
    EXPECT_GE(iter, lsi.begin());
    EXPECT_LT(iter, lsi.end());
    EXPECT_EQ(*iter, i);
  }
}

/// fingerprint tests
template <std::uint8_t fingerprint_size>
void test_fingerprint_lsi(const std::vector<Key> &keys) {
  // build LearnedSecondaryIndex on randomly shuffled keys
  LearnedSecondaryIndex<Key, Model, fingerprint_size> lsi;
  lsi.fit(keys.begin(), keys.end());

  // test that retrieve works for all keys
  for (size_t i = 0; i < keys.size(); i++) {
    const auto iter =
        lsi.template lookup<false>(keys.begin(), keys.end(), keys[i]);
    EXPECT_GE(iter, lsi.begin());
    EXPECT_LT(iter, lsi.end());
    EXPECT_EQ(*iter, i);
  }
};

TEST(LearnedSecondaryIndex, Fingerprinted) {
  const auto datasize = 100000;

  // generate keys
  std::vector<Key> keys;
  keys.reserve(datasize);
  for (size_t i = 0; i < datasize; i++) {
    keys.push_back(i + 20000);
  }

  // shuffle keys (secondary index case)
  std::random_device rd;
  std::mt19937 rng(42);
  std::shuffle(keys.begin(), keys.end(), rng);

  test_fingerprint_lsi<4>(keys);
  test_fingerprint_lsi<8>(keys);
  test_fingerprint_lsi<16>(keys);
}

/// tests for duplicate handling
TEST(LearnedSecondaryIndex, Duplicates) {
  const auto datasize = 100000;

  std::random_device rd;
  std::mt19937 rng(42);

  // generate keys
  std::unordered_map<Key, size_t> key_cnts;
  std::vector<Key> keys;
  keys.reserve(datasize);
  for (size_t i = 0; i < datasize; i++) {
    const auto key = i * i;
    const auto dupl_cnt = rng() % 10 + 1;
    key_cnts[key] = dupl_cnt;
    for (size_t j = 0; j < dupl_cnt; j++) keys.push_back(key);
  }

  // shuffle keys (secondary index case)
  std::shuffle(keys.begin(), keys.end(), rng);

  // build LearnedSecondaryIndex on randomly shuffled keys
  LearnedSecondaryIndex<Key, Model, 0> lsi;
  lsi.fit(keys.begin(), keys.end());

  // test that retrieve works for all keys
  for (size_t i = 0; i < keys.size(); i++) {
    const auto key = keys[i];

    const auto iter =
        lsi.lookup</*lowerbound=*/false>(keys.begin(), keys.end(), key);
    EXPECT_GE(iter, lsi.begin());
    EXPECT_LT(iter, lsi.end());
    EXPECT_EQ(keys[*iter], key);

    // test if is true lower bound
    if (lsi.begin() < iter) EXPECT_NE(keys[*(iter - 1)], key);

    // test if all dupls are correctly found via ++
    size_t j = 0;
    for (; j < key_cnts[key]; j++) EXPECT_EQ(keys[*(iter + j)], key);
    if ((iter + j) < lsi.end()) EXPECT_NE(keys[*(iter + j)], key);
  }
}

template <std::uint8_t fingerprint_size>
void test_fingerprinted_lowerbound() {
  const auto datasize = 100000;

  std::random_device rd;
  std::mt19937 rng(42);

  // generate keys
  std::vector<Key> keys;
  keys.reserve(datasize);
  for (size_t i = 0; i < datasize; i++) {
    const auto key = i * i;
    const auto dupl_cnt = rng() % 10 + 1;
    for (size_t j = 0; j < dupl_cnt; j++) keys.push_back(key);
  }

  // shuffle keys (secondary index case)
  std::shuffle(keys.begin(), keys.end(), rng);

  // build LearnedSecondaryIndex on half of randomly shuffled keys (to not
  // introduce any bias)
  LearnedSecondaryIndex<Key, Model, fingerprint_size> lsi;
  const auto training_end =
      keys.begin() + static_cast<int>(static_cast<double>(keys.size()) * 0.9);
  const auto max_training_elem = *std::max_element(keys.begin(), training_end);
  lsi.fit(keys.begin(), training_end);

  // test that lookup works for keys
  auto it = keys.begin();
  for (; it < training_end; it++) {
    const auto key = *it;
    const auto iter = lsi.template lookup<true>(keys.begin(), keys.end(), key);
    EXPECT_NE(iter, lsi.end());
    EXPECT_EQ(keys[*iter], key);
  }

  // test that lookup works for non-keys
  for (; it < keys.end(); it++) {
    const auto key = *it;
    const auto iter = lsi.template lookup<true>(keys.begin(), keys.end(), key);
    if (key <= max_training_elem) {
      EXPECT_NE(iter, lsi.end());
      EXPECT_LE(key, keys[*iter]);
    } else {
      EXPECT_EQ(iter, lsi.end());
    }
  }
}

/// test for non-key (lowerbound) support
TEST(LearnedSecondaryIndex, LowerBound) {
  test_fingerprinted_lowerbound<0>();
  test_fingerprinted_lowerbound<4>();
}

}  // namespace lsi_tests
