#ifndef POINT_HH
#define POINT_HH

#include "util.hh"

class Point
{
private:
  idx i, j;

public:
  Point(idx i, idx j)
    : i(i), j(j)
  {}

  Point()
    : i(0), j(0)
  {}

  Point(const Point& other) = default;

  Point& operator=(const Point& other) = default;

  idx get_i() const
  {
    return i;
  }

  idx get_j() const
  {
    return j;
  }

  bool operator==(const Point& other) const
  {
    return get_i() == other.get_i() &&
      get_j() == other.get_j();
  }


};

namespace std
{
  /**
   * A hash function for edges
   */
  template<> struct hash<Point>
  {
    typedef Point argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& point) const
    {
      result_type seed = 0;
      hash_combination(seed, point.get_i());
      hash_combination(seed, point.get_j());
      return seed;
    }
  };
}

#endif /* POINT_HH */
