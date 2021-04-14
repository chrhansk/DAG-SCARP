#ifndef SCARP_PROGRAM_HH
#define SCARP_PROGRAM_HH

#include <memory>

#include <set>
#include <unordered_set>

#include "scarp_label.hh"

#include "controls.hh"

#include "cost_function.hh"

class SCARPProgram
{
public:
  typedef std::unordered_set<SCARPLabelPtr,
                             SCARPLabelHash,
                             SCARPLabelComparator> LabelSet;

  typedef std::vector<LabelSet> LabelFront;
private:
  const Graph& graph;
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

  double get_costs(SCARPLabelPtr label);

  std::vector<idx> get_control_sums(SCARPLabelPtr label);

  idx num_labels;

  VertexMap<Controls> get_controls(SCARPLabelPtr label) const;

  double label_distance(SCARPLabelPtr label) const;

public:
  SCARPProgram(const Graph& graph,
               const CostFunction& costs,
               const VertexMap<Controls>& fractional_controls,
               bool vanishing_constraints = false);

  VertexMap<Controls> solve();
};


#endif /* SCARP_PROGRAM_HH */
