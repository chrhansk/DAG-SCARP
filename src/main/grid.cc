#include "grid.hh"

idx compute_grid_length(const Graph& graph,
                        const VertexMap<Point>& coordinates)
{
  idx min_i = std::numeric_limits<idx>::max();
  idx max_i = std::numeric_limits<idx>::min();

  idx min_j = std::numeric_limits<idx>::max();
  idx max_j = std::numeric_limits<idx>::min();


  for(const auto& vertex : graph.get_vertices())
  {
    const auto& coordinate = coordinates(vertex);

    max_i = std::max(max_i, coordinate.get_i());
    min_i = std::min(min_i, coordinate.get_i());

    max_j = std::max(max_j, coordinate.get_j());
    min_j = std::min(min_j, coordinate.get_j());
  }

  return 1 + std::max(max_i - min_i, max_j - min_j);
}
