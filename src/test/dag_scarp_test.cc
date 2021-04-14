#include <gtest/gtest.h>

#include "grid.hh"
#include "scarp/scarp_program.hh"

#include "test_fixture.hh"

TEST_F(TestInstances, test_scarp_solve)
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

    SCARPProgram program(result.graph,
                         costs,
                         result.fractional_controls);

    auto scarp_controls = program.solve();

    const double upper_bound = max_control_deviation(dimension);

    const double distance = control_distance(result.graph,
                                             result.fractional_controls,
                                             scarp_controls);

    EXPECT_TRUE(controls_are_convex(result.graph,
                                    scarp_controls));

    EXPECT_TRUE(controls_are_integral(result.graph,
                                      scarp_controls));

    EXPECT_TRUE(cmp::le(distance, upper_bound));
  }
}
