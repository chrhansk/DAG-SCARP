#include <fstream>

#include "control_reader.hh"
#include "control_writer.hh"
#include "log.hh"

#include "exact_scarp/exact_program.hh"

int main(int argc, char *argv[])
{
  log_init();

  std::ifstream input(argv[1]);
  std::ofstream output(argv[2]);

  auto result = read_file(input);

  auto costs = VariationalCosts();

  ExactProgram program(result.graph,
                       costs,
                       result.fractional_controls);

  auto scarp_controls = program.solve();

  const Vertex source = *result.graph.get_vertices().begin();
  const idx dimension = result.fractional_controls(source).size();

  const double upper_bound = max_control_deviation(dimension);

  const double distance = control_distance(result.graph,
                                           result.fractional_controls,
                                           scarp_controls);

  const double control_cost = costs.evaluate(result.graph, scarp_controls);

  Log(info) << "Distance between controls: "
            << distance
            << ", upper bound: "
            << upper_bound
            << ", control costs: "
            << control_cost;

  write_controls(result.graph,
                 scarp_controls,
                 result.coordinates,
                 output);

  return 0;
}
