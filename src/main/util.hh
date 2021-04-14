#ifndef UTIL_HH
#define UTIL_HH

#include <functional>

typedef unsigned int idx;

constexpr bool debugging_enabled()
{
#if defined(NDEBUG)
  return false;
#else
  return true;
#endif
}

template <class T>
inline void hash_combination(std::size_t & seed, const T & v)
{
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

extern double inf;

#endif /* UTIL_HH */
