#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "include/convenience/builtins.hpp"

namespace dataset {
template <class T> static void sort(std::vector<T> &vec) {
  std::sort(vec.begin(), vec.end());
  // vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
  // vec.shrink_to_fit();
}

/**
 * Loads the datasets values into memory
 * @return a sorted and deduplicated list of all members of the dataset
 */
template <class Key> std::vector<Key> load(const std::string &filepath) {
  std::cout << "loading dataset " << filepath << std::endl;

  // parsing helper functions
  auto read_little_endian_8 = [](const std::vector<unsigned char> &buffer,
                                 uint64_t offset) {
    return static_cast<uint64_t>(buffer[offset + 0]) |
           (static_cast<uint64_t>(buffer[offset + 1]) << 8) |
           (static_cast<uint64_t>(buffer[offset + 2]) << (2 * 8)) |
           (static_cast<uint64_t>(buffer[offset + 3]) << (3 * 8)) |
           (static_cast<uint64_t>(buffer[offset + 4]) << (4 * 8)) |
           (static_cast<uint64_t>(buffer[offset + 5]) << (5 * 8)) |
           (static_cast<uint64_t>(buffer[offset + 6]) << (6 * 8)) |
           (static_cast<uint64_t>(buffer[offset + 7]) << (7 * 8));
  };
  auto read_little_endian_4 = [](const std::vector<unsigned char> &buffer,
                                 uint64_t offset) {
    return buffer[offset + 0] | (buffer[offset + 1] << 8) |
           (buffer[offset + 2] << (2 * 8)) | (buffer[offset + 3] << (3 * 8));
  };

  // Read file into memory from disk. Directly map file for more performance
  std::ifstream input(filepath, std::ios::binary | std::ios::ate);
  std::streamsize size = input.tellg();
  input.seekg(0, std::ios::beg);
  if (!input.is_open()) {
    std::cerr << "file '" + filepath + "' does not exist" << std::endl;
    return {};
  }

  const auto max_num_elements = (size - sizeof(std::uint64_t)) / sizeof(Key);
  std::vector<uint64_t> dataset(max_num_elements, 0);
  {
    std::vector<unsigned char> buffer(size);
    if (!input.read(reinterpret_cast<char *>(buffer.data()), size))
      throw std::runtime_error("Failed to read dataset '" + filepath + "'");

    // Parse file
    uint64_t num_elements = read_little_endian_8(buffer, 0);
    assert(num_elements <= max_num_elements);
    switch (sizeof(Key)) {
    case sizeof(std::uint64_t):
      for (uint64_t i = 0; i < num_elements; i++) {
        // 8 byte header, 8 bytes per entry
        uint64_t offset = i * 8 + 8;
        dataset[i] = read_little_endian_8(buffer, offset);
      }
      break;
    case sizeof(std::uint32_t):
      for (uint64_t i = 0; i < num_elements; i++) {
        // 8 byte header, 4 bytes per entry
        uint64_t offset = i * 4 + 8;
        dataset[i] = read_little_endian_4(buffer, offset);
      }
      break;
    default:
      throw std::runtime_error(
          "unimplemented amount of bytes per value in dataset: " +
          std::to_string(sizeof(Key)));
    }
  }

  // remove duplicates from dataset and put it into random order
  sort(dataset);

  return dataset;
}

enum class ID {
  SEQUENTIAL = 0,
  GAPPED_10 = 1,
  UNIFORM = 2,
  FB = 3,
  OSM = 4,
  WIKI = 5,
  NORMAL = 6,
  BOOKS = 7
};

inline std::string name(ID id) {
  switch (id) {
  case ID::SEQUENTIAL:
    return "seq";
  case ID::GAPPED_10:
    return "gap_10";
  case ID::UNIFORM:
    return "uniform";
  case ID::NORMAL:
    return "normal";
  case ID::FB:
    return "fb";
  case ID::OSM:
    return "osm";
  case ID::WIKI:
    return "wiki";
  case ID::BOOKS:
    return "books";
  }
  return "unnamed";
};

template <class Data = std::uint64_t>
std::vector<Data> load_cached(ID id, size_t dataset_size) {
  static std::random_device rd;
  static std::default_random_engine rng(rd());

  // cache generated & sampled datasets to speed up repeated benchmarks
  static std::unordered_map<ID, std::unordered_map<size_t, std::vector<Data>>>
      datasets;

  // cache sosd dataset files to avoid expensive load operations
  static std::vector<Data> ds_fb, ds_osm, ds_wiki, ds_books;

  // return cached (if available)
  const auto id_it = datasets.find(id);
  if (id_it != datasets.end()) {
    const auto ds_it = id_it->second.find(dataset_size);
    if (ds_it != id_it->second.end())
      return ds_it->second;
  }

  // generate (or random sample) in appropriate size
  std::vector<Data> ds(dataset_size, 0);
  switch (id) {
  case ID::SEQUENTIAL: {
    for (size_t i = 0; i < ds.size(); i++)
      ds[i] = i + 20000;
    break;
  }
  case ID::GAPPED_10: {
    std::uniform_int_distribution<size_t> dist(0, 99999);
    for (size_t i = 0, num = 0; i < ds.size(); i++) {
      do
        num++;
      while (dist(rng) < 10000);
      ds[i] = num;
    }
    break;
  }
  case ID::UNIFORM: {
    std::uniform_int_distribution<Data> dist(0, (0x1LLU << 50) - 1);
    for (size_t i = 0; i < ds.size(); i++)
      ds[i] = dist(rng);
    break;
  }
  case ID::NORMAL: {
    const auto mean = 100.0;
    const auto std_dev = 20.0;
    std::normal_distribution<> dist(mean, std_dev);
    for (size_t i = 0; i < ds.size(); i++) {
      // cutoff after 3 * std_dev
      const auto rand_val =
          std::max(mean - 3 * std_dev, std::min(mean + 3 * std_dev, dist(rng)));

      assert(rand_val >= mean - 3 * std_dev);
      assert(rand_val <= mean + 3 * std_dev);

      // rescale to [0, 2^50)
      const auto rescaled = (rand_val - (mean - 3 * std_dev)) * std::pow(2, 50);

      // round
      ds[i] = std::floor(rescaled);
    }
    break;
  }
  case ID::FB: {
    if (ds_fb.empty()) {
      ds_fb = load<Data>("data/fb_200M_uint64");
      std::shuffle(ds_fb.begin(), ds_fb.end(), rng);
    }
    // ds file does not exist
    if (ds_fb.empty())
      return {};

    // sampling this way is only valid since ds_fb is shuffled!
    for (size_t i = 0; i < ds_fb.size() && i < ds.size(); i++)
      ds[i] = ds_fb[i];
    break;
  }
  case ID::OSM: {
    if (ds_osm.empty()) {
      ds_osm = load<Data>("data/osm_cellids_200M_uint64");
      std::shuffle(ds_osm.begin(), ds_osm.end(), rng);
    }
    // ds file does not exist
    if (ds_osm.empty())
      return {};

    // sampling this way is only valid since ds_osm is shuffled!
    for (size_t i = 0; i < ds_osm.size() && i < ds.size(); i++)
      ds[i] = ds_osm[i];
    break;
  }
  case ID::WIKI: {
    if (ds_wiki.empty()) {
      ds_wiki = load<Data>("data/wiki_ts_200M_uint64");
      std::shuffle(ds_wiki.begin(), ds_wiki.end(), rng);
    }
    // ds file does not exist
    if (ds_wiki.empty())
      return {};

    // sampling this way is only valid since ds_wiki is shuffled!
    for (size_t i = 0; i < ds_wiki.size() && i < ds.size(); i++)
      ds[i] = ds_wiki[i];
    break;
  }
  case ID::BOOKS: {
    if (ds_books.empty()) {
      ds_books = load<Data>("data/books_200M_uint64");
      std::shuffle(ds_books.begin(), ds_books.end(), rng);
    }
    // ds file does not exist
    if (ds_books.empty())
      return {};

    // sampling this way is only valid since ds_wiki is shuffled!
    for (size_t i = 0; i < ds_books.size() && i < ds.size(); i++)
      ds[i] = ds_books[i];
    break;
  }
  default:
    throw std::runtime_error(
        "invalid datastet id " +
        std::to_string(static_cast<std::underlying_type<ID>::type>(id)));
  }

  // since std::numeric_limits<Data> is special (e.g., used as Sentinel),
  // systematically remove this from datasets with minimal impact on the
  // underlying distribution.
  for (auto &key : ds)
    if (key == std::numeric_limits<Data>::max())
      key--;

  // deduplicate and sort before caching to avoid additional work in the future
  sort(ds);

  // cache dataset for future use
  const auto it = datasets.find(id);
  if (it == datasets.end()) {
    std::unordered_map<size_t, std::vector<Data>> map;
    map.insert({dataset_size, ds});
    datasets.insert({id, map});
  } else {
    it->second.insert({dataset_size, ds});
  }

  return ds;
}
}; // namespace dataset
