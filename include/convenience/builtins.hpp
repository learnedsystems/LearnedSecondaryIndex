#include "undef.hpp"

/// prevent unused warnings
#define UNUSED(x) (void)(x)

#ifdef __GNUC__
#ifdef NDEBUG
#define forceinline inline __attribute__((always_inline))
#define likely(expr) __builtin_expect((bool)(expr), 1)
#define unlikely(expr) __builtin_expect((bool)(expr), 0)
#else
#define forceinline
#define likely(expr) expr
#define unlikely(expr) expr
#endif

#define neverinline __attribute__((noinline))
#define alignit(bytes) __attribute__((aligned(bytes)))
#define packit __attribute__((packed))
#define prefetchit(address, mode, locality) \
  __builtin_prefetch(address, mode, locality)

#define full_memory_barrier __sync_synchronize
#else
#error "Your compiler is currently not supported"
#endif
