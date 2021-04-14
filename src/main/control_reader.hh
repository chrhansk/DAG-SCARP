#ifndef CONTROL_READER_HH
#define CONTROL_READER_HH

#include <fstream>

#include "controls.hh"
#include "point.hh"
#include "graph/graph.hh"
#include "graph/vertex_map.hh"

struct ReadResult
{
  Graph graph;
  VertexMap<Controls> fractional_controls;
  VertexMap<Point> coordinates;
};

ReadResult read_file(std::istream& input);

#endif /* CONTROL_READER_HH */
