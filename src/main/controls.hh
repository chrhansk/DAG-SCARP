#ifndef CONTROLS_HH
#define CONTROLS_HH

#include <vector>

#include "util.hh"

#include "graph/graph.hh"
#include "graph/vertex_map.hh"
#include "cmp.hh"

typedef std::vector<double> Controls;

double max_control_deviation(idx dimension);

bool controls_are_integral(const Graph& graph,
                           const VertexMap<Controls>& controls,
                           double eps = cmp::eps);

bool controls_are_convex(const Graph& graph,
                         const VertexMap<Controls>& controls,
                         double eps = cmp::eps);


double control_distance(const Graph& graph,
                        const VertexMap<Controls>& first_controls,
                        const VertexMap<Controls>& second_controls);

#endif /* CONTROLS_HH */
