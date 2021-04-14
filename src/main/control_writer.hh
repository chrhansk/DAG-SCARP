#ifndef CONTROL_WRITER_HH
#define CONTROL_WRITER_HH

#include <iostream>

#include "graph/graph.hh"
#include "graph/vertex_map.hh"
#include "controls.hh"
#include "point.hh"

void write_controls(const Graph& graph,
                    const VertexMap<Controls>& controls,
                    VertexMap<Point> coordinates,
                    std::ostream& out);

#endif /* CONTROL_WRITER_HH */
