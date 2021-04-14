#ifndef SUR_HH
#define SUR_HH

#include "cmp.hh"
#include "controls.hh"
#include "cost_function.hh"
#include "graph/graph.hh"
#include "graph/vertex_map.hh"

VertexMap<Controls>
compute_sur_controls(const Graph& graph,
                     const VertexMap<Controls>& fractional_controls,
                     const CostFunction& costs,
                     bool vanishing_constraints = false,
                     double eps = cmp::eps);

#endif /* SUR_HH */
