#include "sur.hh"

#include <stdexcept>

#include "controls.hh"
#include "log.hh"

VertexMap<Controls>
compute_sur_controls(const Graph& graph,
                     const VertexMap<Controls>& fractional_controls,
                     const CostFunction& costs,
                     bool vanishing_constraints,
                     double eps)
{
  const Vertex source = *graph.get_vertices().begin();
  const idx dimension = fractional_controls(source).size();

  Log(info) << "Computing SUR controls for a graph with "
            << graph.get_vertices().size()
            << " vertices and dimension "
            << dimension;

  assert(controls_are_convex(graph, fractional_controls));

  const double upper_bound = max_control_deviation(dimension);

  VertexMap<Controls> sur_controls(graph, Controls(dimension, 0.));

  idx previous_control = 0;

  std::vector<double> fractional_control_sums(dimension, 0.);

  std::vector<idx> control_sums(dimension, 0);

  auto add_fractional_controls = [&](Vertex vertex)
    {
      for(idx i = 0; i < dimension; ++i)
      {
        fractional_control_sums.at(i) += fractional_controls(vertex).at(i);
      }
    };

  for(const Vertex& vertex : graph.get_vertices())
  {
    idx next_control;
    double next_val = -inf;

    add_fractional_controls(vertex);

    for(idx i = 0; i < dimension; ++i)
    {
      double val = fractional_control_sums.at(i) - ((double) control_sums.at(i));

      if(vanishing_constraints)
      {
        if(cmp::zero(fractional_controls(vertex).at(i), eps))
        {
          continue;
        }
      }

      if(val > next_val)
      {
        next_val = val;
        next_control = i;
      }
    }

    sur_controls(vertex).at(next_control) = 1;
    ++control_sums[next_control];

    previous_control = next_control;
  }

  assert(controls_are_integral(graph, sur_controls));
  assert(controls_are_convex(graph, sur_controls));

  return sur_controls;
}
