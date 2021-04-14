#ifndef EXACT_PROGRAM_HH
#define EXACT_PROGRAM_HH

#include <memory>

#include <set>
#include <unordered_set>

#include "exact_label.hh"

#include "controls.hh"

#include "cost_function.hh"

class ExactProgram
{
public:
  typedef std::unordered_set<ExactLabelPtr,
                             ExactLabelHash,
                             ExactLabelComparator> LabelSet;

  typedef std::vector<LabelSet> LabelFront;
private:
  const Graph& graph;
  const VertexMap<idx> prefix_map;

  const CostFunction& costs;
  const VertexMap<Controls>& fractional_controls;

  bool vanishing_constraints;

  const Vertex source;
  const idx dimension;
  const double upper_bound;

  VertexMap<LabelFront> labels;

  std::vector<double> fractional_control_sums;

  void clear();

  void create_initial_labels();

  void expand(Vertex source, Vertex target);

  void expand_all();

  double get_costs(ExactLabelPtr label);

  std::vector<idx> get_control_sums(ExactLabelPtr label);

  idx num_labels;

  VertexMap<Controls> get_controls(ExactLabelPtr label) const;

  double label_distance(ExactLabelPtr label) const;

public:
  ExactProgram(const Graph& graph,
               const CostFunction& costs,
               const VertexMap<Controls>& fractional_controls,
               bool vanishing_constraints = false);

  VertexMap<Controls> solve();
};


#endif /* EXACT_PROGRAM_HH */
