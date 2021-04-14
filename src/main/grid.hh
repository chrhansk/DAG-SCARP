#ifndef GRID_HH
#define GRID_HH

#include "point.hh"
#include "util.hh"
#include "graph/graph.hh"
#include "graph/vertex_map.hh"

idx compute_grid_length(const Graph& graph,
                        const VertexMap<Point>& coordinates);

#endif /* GRID_HH */
