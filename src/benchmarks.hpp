#pragma once

#include <benchmark/benchmark.h>

#include <chrono>
#include <cstdint>
#include <learned_secondary_index.hpp>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>

#include "include/convenience/builtins.hpp"
#include "support/datasets.hpp"
#include "support/probing_set.hpp"

using namespace learned_secondary_index;

using Key = std::uint64_t;
using Payload = std::uint64_t;
const std::vector<std::int64_t> dataset_sizes{200'000'000};
const std::vector<std::int64_t> datasets{
    static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::BOOKS),
    static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::FB),
    static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::OSM),
    static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::WIKI)};
const std::vector<std::int64_t> probe_distributions{
    static_cast<std::underlying_type_t<dataset::ProbingDistribution>>(
        dataset::ProbingDistribution::UNIFORM)};

template <class Index>
static void EqualityProbe(benchmark::State &state) {
  std::random_device rd;
  std::default_random_engine rng(rd());

  const auto dataset_size = state.range(0);
  const auto did = static_cast<dataset::ID>(state.range(1));

  // load dataset
  auto dataset = dataset::load_cached(did, dataset_size);

  if (dataset.empty()) {
    throw std::runtime_error("can't benchmark on empty dataset");
  }

  // probe in random order to limit caching effects
  const auto probing_dist =
      static_cast<dataset::ProbingDistribution>(state.range(2));
  const auto probing_set = dataset::generate_probing_set(dataset, probing_dist);

  // shuffle dataset & build index
  std::shuffle(dataset.begin(), dataset.end(), rng);

  // Ruild index
  const auto start = std::chrono::steady_clock::now();
  Index index(dataset.begin(), dataset.end());
  const auto index_build_time =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::steady_clock::now() - start)
          .count();

  size_t i = 0;
  size_t errors = 0;
  for (auto _ : state) {
    // get next lookup element
    while (unlikely(i >= probing_set.size())) i -= probing_set.size();
    const auto probed = probing_set[i++];

    const auto iter =
        index.template lookup<false>(dataset.begin(), dataset.end(), probed);
    benchmark::DoNotOptimize(iter);

    errors += dataset[*iter] != probed;

    // prevent interleaved execution
    full_memory_barrier();
  }

  if (errors > 0) throw std::runtime_error("kaputt " + std::to_string(errors));

  state.counters["base_data_accesses"] =
      static_cast<double>(index.base_data_accesses());
  state.counters["false_positive_accesses"] =
      static_cast<double>(index.false_positive_accesses());
  state.counters["build_time"] = static_cast<double>(index_build_time);
  state.counters["model_bytes"] = index.model_byte_size();
  state.counters["perm_bytes"] = index.perm_vector_byte_size();
  state.counters["bytes"] = index.byte_size();
  state.SetLabel(Index::name() + ":" + dataset::name(did) + ":" +
                 dataset::name(probing_dist));
}

template <class Index>
static void LowerboundLookup(benchmark::State &state) {
  std::random_device rd;
  std::default_random_engine rng(rd());

  const auto dataset_size = state.range(0);
  const auto did = static_cast<dataset::ID>(state.range(1));

  // load dataset
  auto dataset = dataset::load_cached(did, dataset_size);

  if (dataset.empty()) {
    throw std::runtime_error("can't benchmark on empty dataset");
  }

  // shuffle dataset & build index
  std::shuffle(dataset.begin(), dataset.end(), rng);

  // build index on first k elements of dataset. Since it is uniform randomly
  // shuffled, this won't introduce any unwanted bias
  const int insert_end =
      static_cast<int>(static_cast<double>(dataset.size()) * 0.9);
  const auto start = std::chrono::steady_clock::now();
  Index index(dataset.begin(), dataset.begin() + insert_end);
  const auto index_build_time =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::steady_clock::now() - start)
          .count();

  // probe set is second half of the dataset. It is always uniform random
  decltype(dataset) probing_set(dataset.begin() + insert_end, dataset.end());
  std::shuffle(probing_set.begin(), probing_set.end(), rng);

  size_t i = 0;
  size_t errors = 0;
  for (auto _ : state) {
    // get next lookup element
    while (unlikely(i >= probing_set.size())) i -= probing_set.size();
    const auto probed = probing_set[i++];

    auto lb_iter = index.template lookup<true>(
        dataset.begin(), dataset.begin() + insert_end, probed);

    if (lb_iter != index.end() && dataset[*lb_iter] < probed) {
      errors++;
    }

    // prevent interleaved execution
    full_memory_barrier();
  }

  if (errors > 0) throw std::runtime_error("kaputt " + std::to_string(errors));

  state.counters["base_data_accesses"] =
      static_cast<double>(index.base_data_accesses());
  state.counters["false_positive_accesses"] =
      static_cast<double>(index.false_positive_accesses());
  state.counters["build_time"] = static_cast<double>(index_build_time);
  state.counters["model_bytes"] = index.model_byte_size();
  state.counters["perm_bytes"] = index.perm_vector_byte_size();
  state.counters["bytes"] = index.byte_size();
  state.SetLabel(Index::name() + ":" + dataset::name(did));
}
