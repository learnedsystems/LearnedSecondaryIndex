#pragma once

namespace fast64_tests {

using Key = std::uint64_t;

static constexpr size_t SEED = 42;

TEST(FAST64, E2E) {
  const auto datasize = 100'000;

  // generate keys
  std::vector<Key> keys;
  keys.reserve(datasize);
  for (size_t i = 0; i < datasize; i++) keys.push_back(i + 20'000);

  // shuffle keys (secondary index case)
  std::mt19937 rng(  // NOLINT(cert-msc51-cpp) constant seed for reproducibility
      SEED);
  std::shuffle(keys.begin(), keys.end(), rng);

  // build LearnedSecondaryIndex on randomly shuffled keys
  lsi_competitors::FAST64<Key> fast(keys.begin(), keys.end());

  // test that retrieve works for all keys
  for (size_t i = 0; i < keys.size(); i++) {
    const auto iter = fast.lookup<false>(keys.begin(), keys.end(), keys[i]);
    EXPECT_EQ(*iter, i);
  }
}

}  // namespace fast64_tests
