#include "cost_function.hh"

double CostFunction::operator()(const Edge& edge,
                                const Controls& source_controls,
                                const Controls& target_controls) const
{
  double total_value = 0.;

  const Vertex source = edge.get_source();
  const Vertex target = edge.get_target();

  assert(source_controls.size() == target_controls.size());

  const idx dimension = source_controls.size();

  for(idx i = 0; i < dimension; ++i)
  {
    const double previous_control = source_controls.at(i);
    const double next_control = target_controls.at(i);

    const double cost_value = (*this)(edge, previous_control, next_control);

    total_value += cost_value;
  }

  return total_value;
}

double CostFunction::evaluate(const Graph& graph,
                              const VertexMap<Controls>& controls) const
{
  double total_value = 0.;

  const Vertex source = *graph.get_vertices().begin();
  const idx dimension = controls(source).size();

  for(const Edge& edge : graph.get_edges())
  {
    Vertex source = edge.get_source();
    Vertex target = edge.get_target();

    total_value += (*this)(edge, controls(source), controls(target));
  }

  return total_value;
}
