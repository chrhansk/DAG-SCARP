#include <fstream>
#include <filesystem>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "control_reader.hh"
#include "control_writer.hh"

#include "cost_function.hh"
#include "grid.hh"
#include "log.hh"
#include "timer.hh"

#include "dag_program.hh"
#include "scarp/scarp_program.hh"

int main(int argc, char *argv[])
{
  log_init();

  po::options_description desc("Allowed options");

  std::vector<std::string> input_names;
  std::string output_name;

  bool scarp_heuristic = false;
  bool vanishing_constraints = false;

  desc.add_options()
    ("help", "produce help message")
    ("scarp_heuristic", po::bool_switch(&scarp_heuristic)->default_value(false), "use SCARP has heuristic")
    ("vanishing_constraints", po::bool_switch(&vanishing_constraints)->default_value(false), "enable vanishing constraints")
    ("input", po::value<std::vector<std::string>>(&input_names)->required(), "input file")
    ("output", po::value<std::string>(&output_name)->required(), "output file");

  po::variables_map vm;

  po::positional_options_description positional_options;
  positional_options.add("input", -1);

  po::store(po::command_line_parser(argc, argv)
            .options(desc)
            .positional(positional_options)
            .run(),
            vm);

  if(vm.count("help"))
  {
    std::cerr << "Usage: "
              << argv[0]
              << " [options] <input> <ouput>"
              << std::endl;

    std::cerr << desc << std::endl;

    return 1;
  }

  po::notify(vm);

  std::ofstream output(output_name);

  output << "Name;Objective;Distance;UpperBound;RunningTime" << std::endl;

  for(const std::string& input_name : input_names)
  {
    std::ifstream input(input_name);
    auto result = read_file(input);

    const idx grid_length = compute_grid_length(result.graph,
                                                result.coordinates);

    const double scale_factor = 1. / ((double) grid_length);

    auto costs = VariationalCosts(scale_factor);

    const Vertex source = *result.graph.get_vertices().begin();
    const idx dimension = result.fractional_controls(source).size();

    const double upper_bound = max_control_deviation(dimension);

    Timer timer;

    DAGProgram program(result.graph,
                       result.fractional_controls,
                       costs);

    if(scarp_heuristic)
    {
      SCARPProgram scarp_program(result.graph,
                                 costs,
                                 result.fractional_controls);

      auto scarp_controls = scarp_program.solve();

      const double control_cost = costs.evaluate(result.graph, scarp_controls);

      Log(info) << "Costs of initial solution: "
                << control_cost;

      program.set_initial_solution(scarp_controls);
    }

    auto mip_controls = program.solve();

    const double elapsed = timer.elapsed();

    const double distance = control_distance(result.graph,
                                             result.fractional_controls,
                                             mip_controls);

    const double control_cost = costs.evaluate(result.graph, mip_controls);

    Log(info) << "Distance between controls: "
              << distance
              << ", upper bound: "
              << upper_bound
              << ", control costs: "
              << control_cost;

    /*
    write_controls(result.graph,
                   mip_controls,
                   result.coordinates,
                   output);
    */

    std::string stem = std::filesystem::path(input_name).stem();

    output << stem << ";"
           << control_cost << ";"
           << distance << ";"
           << upper_bound << ";"
           << elapsed
           << std::endl;
  }

  return 0;
}
