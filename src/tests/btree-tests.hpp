#pragma once

#include <gtest/gtest.h>

#include <algorithm>
#include <random>

#include "src/competitors.hpp"

namespace btree_tests {
using Key = std::uint64_t;

TEST(BTree, E2E) {
  const auto datasize = 100000;

  // generate keys
  std::vector<Key> keys;
  keys.reserve(datasize);
  for (size_t i = 0; i < datasize; i++) keys.push_back(i + 20000);

  // shuffle keys (secondary index case)
  std::random_device rd;
  std::mt19937 rng(rd());
  std::shuffle(keys.begin(), keys.end(), rng);

  // build LearnedSecondaryIndex on randomly shuffled keys
  lsi_competitors::BTree<Key> tree;
  tree.fit(keys.begin(), keys.end());

  // test that retrieve works for all keys
  for (size_t i = 0; i < keys.size(); i++) {
    const auto iter = tree.lookup<false>(keys.begin(), keys.end(), keys[i]);
    EXPECT_EQ(*iter, i);
  }
}

TEST(BTree, Bulkload) {
  const auto datasize = 100000;

  // generate keys
  std::vector<Key> keys;
  keys.reserve(datasize);
  for (size_t i = 0; i < datasize; i++) keys.push_back(i + 20000);

  // shuffle keys (secondary index case)
  std::random_device rd;
  std::mt19937 rng(rd());
  std::shuffle(keys.begin(), keys.end(), rng);

  // build LearnedSecondaryIndex on randomly shuffled keys
  lsi_competitors::BTree<Key, true> tree;
  tree.fit(keys.begin(), keys.end());

  // test that retrieve works for all keys
  for (size_t i = 0; i < keys.size(); i++) {
    const auto iter = tree.lookup<false>(keys.begin(), keys.end(), keys[i]);
    EXPECT_EQ(*iter, i);
  }
}

TEST(BTree, Duplicates) {
  const auto datasize = 100000;

  std::mt19937 rng(1337);

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

  // build BTree on randomly shuffled keys
  lsi_competitors::BTree<Key> btree;
  btree.fit(keys.begin(), keys.end());

  // test that retrieve works for all keys
  for (size_t i = 0; i < keys.size(); i++) {
    const auto key = keys[i];

    const auto iter = btree.lookup<false>(keys.begin(), keys.end(), key);
    EXPECT_NE(iter, btree.end());
    EXPECT_EQ(keys[*iter], key);

    // test if all dupls are correctly found via ++
    auto it = iter;
    for (size_t j = 0; j < key_cnts[key]; j++, it++) EXPECT_EQ(keys[*it], key);
    if (it != btree.end()) EXPECT_NE(keys[*it], key);
  }
}

TEST(BTree, LowerBound) {
  const auto datasize = 100000;

  std::default_random_engine rng(42);

  // generate keys
  std::vector<Key> keys;
  keys.reserve(datasize);
  for (size_t i = 0; i < datasize; i++) keys.push_back(i + 20000);

  // shuffle keys (secondary index case)
  std::shuffle(keys.begin(), keys.end(), rng);

  // build BTree on half of randomly shuffled keys (sample!)
  lsi_competitors::BTree<Key> btree;
  const auto training_end =
      keys.begin() + static_cast<int>(static_cast<double>(keys.size()) * 0.9);
  const auto max_training_elem = *std::max_element(keys.begin(), training_end);
  btree.fit(keys.begin(), training_end);

  // test that lookup works for keys
  auto it = keys.begin();
  for (; it < training_end; it++) {
    const auto key = *it;
    const auto iter = btree.lookup<true>(keys.begin(), keys.end(), key);
    EXPECT_NE(iter, btree.end());
    EXPECT_EQ(keys[*iter], key);
  }

  // test that lookup works for non-keys
  for (; it < keys.end(); it++) {
    const auto key = *it;
    const auto iter = btree.lookup<true>(keys.begin(), keys.end(), key);
    if (key <= max_training_elem) {
      EXPECT_NE(iter, btree.end());
      EXPECT_LT(key, keys[*iter]);
    } else {
      EXPECT_EQ(iter, btree.end());
    }
  }
}
}  // namespace btree_tests
