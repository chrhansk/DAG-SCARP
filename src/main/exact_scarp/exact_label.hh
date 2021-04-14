#ifndef EXACT_LABEL_HH
#define EXACT_LABEL_HH

#include <optional>
#include <memory>

#include <boost/container_hash/hash.hpp>

#include "util.hh"
#include "graph/graph.hh"

class ExactLabel;

typedef std::shared_ptr<ExactLabel> ExactLabelPtr;

class ExactLabel
{
private:
  ExactLabelPtr predecessor;

  std::vector<idx> control_sums;
  idx current_control;
  double cost;
  Vertex vertex;

  std::vector<std::optional<idx>> prefix;
public:

  ExactLabel(idx current_control,
             Vertex vertex,
             double cost,
             idx prefix_length,
             ExactLabelPtr predecessor);

  ExactLabel(idx current_control,
             Vertex vertex,
             idx dimension,
             idx prefix_length,
             double cost = 0.);

  void replace(const ExactLabel& other);

  bool operator==(const ExactLabel& other) const;

  const std::vector<std::optional<idx>>& get_prefix() const
  {
    return prefix;
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

  double get_cost() const
  {
    return cost;
  }

  ExactLabelPtr get_predecessor() const
  {
    return predecessor;
  }
};

namespace std
{
  template<>
  struct hash<ExactLabel>
  {
    std::size_t operator()(const ExactLabel& label) const
    {
      std::size_t seed = 0;

      for(const auto& vertex : label.get_prefix())
      {
        hash_combination(seed, vertex);
      }

      for(const auto& value : label.get_control_sums())
      {
        hash_combination(seed, value);
      }

      return seed;
    }
  };
}

struct ExactLabelHash
{
  std::size_t operator()(const ExactLabelPtr& label) const
  {
    assert(label);
    return std::hash<ExactLabel>()(*label);
  }
};

struct ExactLabelComparator
{
  bool operator()(const ExactLabelPtr& first,
                  const ExactLabelPtr& second) const
  {
    return *first == *second;
  }
};


#endif /* EXACT_LABEL_HH */
