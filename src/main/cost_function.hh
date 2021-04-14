#ifndef COST_FUNCTION_HH
#define COST_FUNCTION_HH

#include "controls.hh"
#include "util.hh"
#include "graph/graph.hh"
#include "graph/vertex_map.hh"

class CostFunction
{
public:
  virtual double operator()(const Edge& edge,
                            double previous_control,
                            double current_control) const = 0;

  virtual double operator()(const Edge& edge,
                            const Controls& previous_controls,
                            const Controls& current_controls) const;

  double evaluate(const Graph& graph,
                  const VertexMap<Controls>& controls) const;
};

class VariationalCosts : public CostFunction
{
private:
  const double scale_factor;
public:
  VariationalCosts(double scale_factor = 1.)
    : scale_factor(scale_factor)
  {}

  double operator()(const Edge& edge,
                    double previous_control,
                    double current_control) const override
  {
    return 0.5 * scale_factor * std::abs(previous_control - current_control);
  }

};

#endif /* COST_FUNCTION_HH */
