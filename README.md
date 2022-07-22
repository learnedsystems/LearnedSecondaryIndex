# LearnedSecondaryIndex

## Overview 
This repo is the main codebase for our paper _LSI: A Learned Secondary Index Structure_.

Learned Secondary Index (LSI) is a first attempt to use learned indexes for indexing unsorted data.
LSI works by building a learned index over a permutation vector,
which allows binary search to performed on the unsorted base data using random access.
We additionally augment LSI with a fingerprint vector to accelerate equality lookups. 

## Usage

Execute `./test.sh` to run our testcases and `./run.sh` to run our benchmarks.
Note that you may need to edit the `.env` file first to contain the correct path to your compiler.
Any recent version of clang should work.

Run all cells in `paper_plots.ipynb` to recreate the plots in `results/`.

### CMake

You can include `lsi` in your own CMake based project like this:
``` lsi
include(FetchContent)
FetchContent_Declare(
    lsi
    GIT_REPOSITORY "https://github.com/learnedsystems/LearnedSecondaryIndex"
    GIT_TAG main
)
FetchContent_MakeAvailable(lsi)

target_link_libraries(your_target lsi)
```

## Structure

- `include/` contains the code newly contributed by our work
- `src/` contains tests, benchmark driver code and competitors
- `results/` contains the results referenced in the paper and the accompanying plots

## Cite

Please cite our [aiDM@SIGMOD 2022 paper](https://dl.acm.org/doi/TODO) if you use this code in your own work, e.g.:

<!-- 
@inproceedings{lsi,
  author    = {Andreas Kipf and
               Dominik Horn and
               Pascal Pfeil and
               Ryan Marcus and
               Tim Kraska},
  title     = {{LSI}: A Learned Secondary Index Structure},

  #TODO

}
-->

```
@misc{https://doi.org/10.48550/arxiv.2205.05769,
  doi = {10.48550/ARXIV.2205.05769},
  url = {https://arxiv.org/abs/2205.05769},
  author = {Kipf, Andreas and Horn, Dominik and Pfeil, Pascal and Marcus, Ryan and Kraska, Tim},
  keywords = {Databases (cs.DB), Machine Learning (cs.LG), FOS: Computer and information sciences, FOS: Computer and information sciences},
  title = {LSI: A Learned Secondary Index Structure},
  publisher = {arXiv},
  year = {2022}, 
  copyright = {arXiv.org perpetual, non-exclusive license}
}
```
