#include "controls.hh"

double max_control_deviation(idx dimension)
{
  double max_deviation = 0.;

  for(idx i = 2; i <= dimension; ++i)
  {
    max_deviation += 1. / (double(i));
  }

  return max_deviation;
}

bool controls_are_integral(const Graph& graph,
                           const VertexMap<Controls>& controls,
                           double eps)
{
  for(const Vertex& vertex : graph.get_vertices())
  {
    for(const double& value : controls(vertex))
    {
      if(!cmp::integral(value, eps))
      {
        return false;
      }
    }
  }

  return true;
}

bool controls_are_convex(const Graph& graph,
                         const VertexMap<Controls>& controls,
                         double eps)
{
  for(const Vertex& vertex : graph.get_vertices())
  {
    double sum = 0.;

    for(const double& value : controls(vertex))
    {
      if(cmp::neg(value, eps))
      {
        return false;
      }

      sum += value;
    }

    if(!cmp::eq(sum, 1., eps))
    {
      return false;
    }
  }

  return true;
}

double control_distance(const Graph& graph,
                        const VertexMap<Controls>& first_controls,
                        const VertexMap<Controls>& second_controls)
{
  double max_deviation = -inf;

  Vertex source = *graph.get_vertices().begin();

  const idx dimension = first_controls(source).size();

  std::vector<double> first_sum(dimension, 0.);
  std::vector<double> second_sum(dimension, 0.);

  for(const Vertex& vertex : graph.get_vertices())
  {
    for(idx i = 0; i < dimension; ++i)
    {
      first_sum.at(i) += first_controls(vertex).at(i);
      second_sum.at(i) += second_controls(vertex).at(i);
    }

    for(idx i = 0; i < dimension; ++i)
    {
      double deviation = std::fabs(first_sum.at(i) - second_sum.at(i));

      if(deviation > max_deviation)
      {
        max_deviation = deviation;
      }
    }
  }

  return max_deviation;
}
