#include <fstream>

#include "control_reader.hh"
#include "control_writer.hh"
#include "grid.hh"

#include "cost_function.hh"
#include "log.hh"

#include "dag_program.hh"

int main(int argc, char *argv[])
{
  log_init();

  std::ifstream input(argv[1]);
  std::ofstream output(argv[2]);

  auto result = read_file(input);

  const idx grid_length = compute_grid_length(result.graph,
                                              result.coordinates);

  const double scale_factor = 1. / ((double) grid_length);

  auto costs = VariationalCosts(scale_factor);

  try
  {
    DAGProgram program(result.graph,
                       result.fractional_controls,
                       costs);

    auto mip_controls = program.solve();

    write_controls(result.graph,
                   mip_controls,
                   result.coordinates,
                   output);
  }
  catch(GRBException exc)
  {
    Log(error) << "Caught Gurobi exception: " << exc.getMessage();
    return 1;
  }


  return 0;
}
