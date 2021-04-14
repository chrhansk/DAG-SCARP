#ifndef SCARP_LABEL_HH
#define SCARP_LABEL_HH

#include <optional>
#include <boost/container_hash/hash.hpp>

#include "util.hh"
#include "graph/graph.hh"

class SCARPLabel;

typedef std::shared_ptr<SCARPLabel> SCARPLabelPtr;

class SCARPLabel
{
private:
  SCARPLabelPtr predecessor;
  Vertex vertex;

  std::vector<idx> control_sums;
  idx current_control;
  double cost;

public:
  SCARPLabel(idx current_control,
             Vertex vertex,
             double cost,
             SCARPLabelPtr predecessor)
    : predecessor(predecessor),
      vertex(vertex),
      control_sums(predecessor->get_control_sums()),
      current_control(current_control),
      cost(cost)
  {
    control_sums.at(current_control)++;
  }

  SCARPLabel(idx current_control,
             Vertex vertex,
             idx dimension,
             double cost = 0.)
    : vertex(vertex),
      control_sums(dimension, 0),
      current_control(current_control),
      cost(cost)
  {
    control_sums.at(current_control)++;
  }

  void replace(const SCARPLabel& other)
  {
    assert(vertex == other.get_vertex());
    assert(current_control == other.get_current_control());

    control_sums = other.get_control_sums();
    cost = other.get_cost();
    predecessor = other.get_predecessor();
  }

  bool operator==(const SCARPLabel& other) const
  {
    return control_sums == other.control_sums;
  }

  const std::vector<idx>& get_control_sums() const
  {
    return control_sums;
  }

  idx get_current_control() const
  {
    return current_control;
  }

  Vertex get_vertex() const
  {
    return vertex;
  }

  bool operator<(const SCARPLabel& other) const
  {
    for(idx i = 0; i < control_sums.size(); ++i)
    {
      if(control_sums[i] != other.control_sums[i])
      {
        return control_sums[i] < other.control_sums[i];
      }
    }

    return false;
  }

  double get_cost() const
  {
    return cost;
  }

  SCARPLabelPtr get_predecessor() const
  {
    return predecessor;
  }

};

namespace std
{
  template<>
  struct hash<SCARPLabel>
  {
    std::size_t operator()(const SCARPLabel& label) const
    {
      std::size_t seed = 0;

      boost::hash_combine(seed,
                          boost::hash_range(label.get_control_sums().begin(),
                                            label.get_control_sums().end()));

      boost::hash_combine(seed, label.get_current_control());

      return seed;
    }
  };
}

struct SCARPLabelHash
{
  std::size_t operator()(const SCARPLabelPtr& label) const
  {
    assert(label);
    return std::hash<SCARPLabel>()(*label);
  }
};

struct SCARPLabelOrdering
{
  std::size_t operator()(const SCARPLabelPtr& first,
                         const SCARPLabelPtr& second) const
  {
    assert(first);
    assert(second);

    return *first < *second;
  }
};

struct SCARPLabelComparator
{
  bool operator()(const SCARPLabelPtr& first,
                         const SCARPLabelPtr& second) const
  {
    return *first == *second;
  }
};

#endif /* SCARP_LABEL_HH */
