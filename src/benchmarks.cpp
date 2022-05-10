#include "benchmarks.hpp"

#include <learned_hashing.hpp>
#include <learned_secondary_index.hpp>

#include "src/competitors.hpp"
#include "src/support/datasets.hpp"

// General
#define SINGLE_ARG(...) __VA_ARGS__
#define BM_EQ(Index)                                                    \
  BENCHMARK_TEMPLATE(EqualityProbe, Index)                              \
      ->ArgsProduct({dataset_sizes,                                     \
                     {static_cast<std::underlying_type_t<dataset::ID>>( \
                         dataset::ID::BOOKS)},                          \
                     probe_distributions})                              \
      ->Iterations(10000000);
#define BM_LOWER_BOUND(Index)                                       \
  BENCHMARK_TEMPLATE(LowerboundLookup, Index)                       \
      ->ArgsProduct({dataset_sizes, datasets, probe_distributions}) \
      ->Iterations(10000000);
#define BM(Index)          \
  BM_EQ(SINGLE_ARG(Index)) \
  BM_LOWER_BOUND(SINGLE_ARG(Index))
#define BM_LSI_LOWER_BOUND(Model) \
  BM_LOWER_BOUND(SINGLE_ARG(      \
      learned_secondary_index::LearnedSecondaryIndex<Key, Model, 0>))

/// Experiment 1: Lowerbound
BM_LOWER_BOUND(SINGLE_ARG(lsi_competitors::BTree<Key, false>))
BM_LOWER_BOUND(SINGLE_ARG(lsi_competitors::BTree<Key, true>))
BM_LOWER_BOUND(SINGLE_ARG(lsi_competitors::ART<Key>))
BM_LSI_LOWER_BOUND(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 1>))
BM_LSI_LOWER_BOUND(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 4>))
BM_LSI_LOWER_BOUND(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 8>))

/// Experiment 2: Equality
BM_EQ(lsi_competitors::RobinHash<Key>)
BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
                 Key, learned_hashing::TrieSplineHash<Key, 1>, 8>))
BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
                 Key, learned_hashing::TrieSplineHash<Key, 8>, 8>))

/// Experiment 3: Equality fingerprint - All measured as part of exp 5
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 1>, 0>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 4>, 0>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 16>, 0>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 64>, 0>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 256>, 0>))
//
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 1>, 2>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 4>, 2>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 16>, 2>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 64>, 2>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 256>, 2>))
//
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 1>, 4>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 4>, 4>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 16>, 4>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 64>, 4>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 256>, 4>))

// Already measured as part of exp 1
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 1>, 8>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 4>, 8>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 16>, 8>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 64>, 8>))
// BM_EQ(SINGLE_ARG(learned_secondary_index::LearnedSecondaryIndex<
//                  Key, learned_hashing::TrieSplineHash<Key, 256>, 8>))

/// Experiment 4: Using CHT
#define EXP_4(Model)                                                      \
  BENCHMARK_TEMPLATE(                                                     \
      LowerboundLookup,                                                   \
      SINGLE_ARG(                                                         \
          learned_secondary_index::LearnedSecondaryIndex<Key, Model, 0>)) \
      ->ArgsProduct({dataset_sizes,                                       \
                     {static_cast<std::underlying_type_t<dataset::ID>>(   \
                         dataset::ID::BOOKS)},                            \
                     probe_distributions})                                \
      ->Iterations(10000000);

EXP_4(SINGLE_ARG(learned_hashing::CHTHash<Key, 1, 64>))
EXP_4(SINGLE_ARG(learned_hashing::CHTHash<Key, 4, 64>))
EXP_4(SINGLE_ARG(learned_hashing::CHTHash<Key, 8, 64>))
EXP_4(SINGLE_ARG(learned_hashing::CHTHash<Key, 16, 64>))
// Already measured as part of exp 1
// EXP_4(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 4>))
// EXP_4(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 8>))
EXP_4(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 16>))

/// Experiment 5: Model error & Fingerprint size Heatmap
#define EXP_5(Model)                                                  \
  BM_EQ(SINGLE_ARG(                                                   \
      learned_secondary_index::LearnedSecondaryIndex<Key, Model, 0>)) \
  BM_EQ(SINGLE_ARG(                                                   \
      learned_secondary_index::LearnedSecondaryIndex<Key, Model, 1>)) \
  BM_EQ(SINGLE_ARG(                                                   \
      learned_secondary_index::LearnedSecondaryIndex<Key, Model, 2>)) \
  BM_EQ(SINGLE_ARG(                                                   \
      learned_secondary_index::LearnedSecondaryIndex<Key, Model, 4>)) \
  BM_EQ(SINGLE_ARG(                                                   \
      learned_secondary_index::LearnedSecondaryIndex<Key, Model, 8>)) \
  BM_EQ(SINGLE_ARG(                                                   \
      learned_secondary_index::LearnedSecondaryIndex<Key, Model, 16>))

EXP_5(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 1>))
EXP_5(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 2>))
EXP_5(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 4>))
EXP_5(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 8>))
EXP_5(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 16>))
EXP_5(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 32>))
EXP_5(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 64>))
EXP_5(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 128>))
EXP_5(SINGLE_ARG(learned_hashing::TrieSplineHash<Key, 256>))

BENCHMARK_MAIN();
