import math
import pandas as pd


def min_bits_bitpacking(n: int):
    """ If we merely bitpack."""
    return math.log2(n)


def min_bits_optimal(n: int):
    """ The information theory lower bound is log(n!) / n.

    To make this easier to compute, we use the following equation:
        log(ab) = log(a) + log(b)
    That means that
        log(n!) = log(n) + log(n-1) + ... + log(1)
    """
    log_n_fac = sum([math.log2(i) for i in range(1, n + 1)])
    return log_n_fac / n


def min_bits_permutation(n: int):
    """
    https://github.com/RyanMarcus/permutation_compression
    1_000_000 numbers 32 bit
    uncompressed 32_000_000 bit = 4_000_000 byte = 4MB
    compression ratio 1.684
    compressed 32_000_000 bit / 1.684 = 19_002_375 bit = 2_375_297 byte = 2.4MB
    19_002_375 / 1_000_000 = 19 bits per element
    log2(1_000_000) - c = 19 <-> c ≈ 0.93
    """
    return math.log2(n) - 0.93


def min_bits_chia(n: int):
    """
    https://hackmd.io/@dabo/rkP8Pcf9t
    The authors state that in practice using q=32 is sufficient. This results in the factor of 1.34.
    Note that this might be different in our case, and we could get arbitrarily close to the lower bound.
    """
    return math.log2(n) - 1.34


if __name__ == '__main__':
    data = []

    for exp in range(2, 10):
        size = 10 ** exp
        data.append(['optimal', size, min_bits_optimal(size)])
        data.append(['chia', size, min_bits_chia(size)])
        data.append(['permutation', size, min_bits_permutation(size)])
        data.append(['bitpacking', size, min_bits_bitpacking(size)])

    df = pd.DataFrame(data, columns=['method', 'num_keys', 'bits_per_key'])
    df.to_csv('simulate.csv', index=False)
