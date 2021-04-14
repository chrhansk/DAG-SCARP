#include <gtest/gtest.h>

#include "grid.hh"
#include "dag_program.hh"

#include "test_fixture.hh"

TEST_F(TestInstances, test_mip_solve)
{
  for(const auto& test_instance : test_instances)
  {
    auto result = test_instance.read();

    ASSERT_TRUE(controls_are_convex(result.graph,
                                    result.fractional_controls));

    const Vertex source = *result.graph.get_vertices().begin();
    const idx dimension = result.fractional_controls(source).size();

    const idx grid_length = compute_grid_length(result.graph,
                                                result.coordinates);

    const double scale_factor = 1. / ((double) grid_length);

    auto costs = VariationalCosts(scale_factor);

    DAGProgram program(result.graph,
                       result.fractional_controls,
                       costs);

    auto mip_controls = program.solve();

    const double upper_bound = max_control_deviation(dimension);

    const double distance = control_distance(result.graph,
                                             result.fractional_controls,
                                             mip_controls);

    const double control_cost = costs.evaluate(result.graph, mip_controls);

    EXPECT_TRUE(controls_are_convex(result.graph,
                                    mip_controls));

    EXPECT_TRUE(controls_are_integral(result.graph,
                                      mip_controls));

    EXPECT_TRUE(cmp::le(distance, upper_bound));

    EXPECT_TRUE(cmp::eq(control_cost, test_instance.get_optimal_objective()));
  }
}
