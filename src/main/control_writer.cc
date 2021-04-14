#include "control_writer.hh"

#include <iomanip>

void write_controls(const Graph& graph,
                    const VertexMap<Controls>& controls,
                    VertexMap<Point> coordinates,
                    std::ostream& out)
{
  for(const Vertex& vertex: graph.get_vertices())
  {
    Point point = coordinates(vertex);

    out << point.get_i() << "," << point.get_j();

    for(const double& value : controls(vertex))
    {
      out << "," << std::fixed << value;
    }

    out << std::endl;
  }
}
