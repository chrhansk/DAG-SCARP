#include <gtest/gtest.h>

#include "grid.hh"
#include "sur/sur.hh"

#include "test_fixture.hh"

TEST_F(TestInstances, test_sur_solve)
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

    auto sur_controls = compute_sur_controls(result.graph,
                                             result.fractional_controls,
                                             costs);

    const double upper_bound = max_control_deviation(dimension);

    const double distance = control_distance(result.graph,
                                             result.fractional_controls,
                                             sur_controls);

    EXPECT_TRUE(controls_are_convex(result.graph,
                                    sur_controls));

    EXPECT_TRUE(controls_are_integral(result.graph,
                                      sur_controls));

    EXPECT_TRUE(cmp::le(distance, upper_bound));
  }
}
