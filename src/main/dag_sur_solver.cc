#include <fstream>

#include "control_reader.hh"
#include "control_writer.hh"

#include "cost_function.hh"
#include "log.hh"
#include "sur/sur.hh"

int main(int argc, char *argv[])
{
  log_init();

  std::ifstream input(argv[1]);
  std::ofstream output(argv[2]);

  auto result = read_file(input);

  auto costs = VariationalCosts();

  auto sur_controls = compute_sur_controls(result.graph,
                                           result.fractional_controls,
                                           costs);

  const Vertex source = *result.graph.get_vertices().begin();
  const idx dimension = result.fractional_controls(source).size();

  const double upper_bound = max_control_deviation(dimension);

  const double distance = control_distance(result.graph,
                                           result.fractional_controls,
                                           sur_controls);

  const double control_cost = costs.evaluate(result.graph, sur_controls);

  Log(info) << "Distance between controls: "
            << distance
            << ", upper bound: "
            << upper_bound
            << ", control costs: "
            << control_cost;

  write_controls(result.graph,
                 sur_controls,
                 result.coordinates,
                 output);

  return 0;
}
