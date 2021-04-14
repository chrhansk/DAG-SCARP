#include "dag_program.hh"

#include <sstream>

#include "cmp.hh"
#include "controls.hh"
#include "log.hh"

DAGProgram::DAGProgram(const Graph& graph,
                       const VertexMap<Controls>& fractional_controls,
                       const CostFunction& costs)
  : model(env),
    graph(graph),
    fractional_controls(fractional_controls),
    costs(costs),
    variables(graph, {}),
    source(*graph.get_vertices().begin()),
    dimension(fractional_controls(source).size()),
    upper_bound(max_control_deviation(dimension)),
    cost_variables(graph, std::vector<CostVariable>{})
{
  assert(controls_are_convex(graph, fractional_controls));

  create_variables();
  add_vertex_constraints();
  add_approximation_constraints();
  add_objective();
}

void DAGProgram::create_variables()
{
  Log(debug) << "Creating variables";

  for(const Vertex& vertex : graph.get_vertices())
  {
    for(idx i = 0; i < dimension; ++i)
    {
      std::ostringstream namebuf;
      namebuf << "x_" << vertex.get_index() << "_" << i;

      GRBVar var = model.addVar(0.,
                                1.,
                                0.,
                                GRB_BINARY,
                                namebuf.str());

      variables(vertex).push_back(var);
    }
  }
}

void DAGProgram::add_vertex_constraints()
{
  Log(debug) << "Adding vertex constraints";

  for(const Vertex& vertex : graph.get_vertices())
  {
    GRBLinExpr sum;

    for(const auto& variable : variables(vertex))
    {
      sum += variable;
    }

    std::ostringstream namebuf;

    namebuf << "one_" << vertex.get_index();

    model.addConstr(sum == 1, namebuf.str());
  }
}

void DAGProgram::add_approximation_constraints()
{
  Log(debug) << "Adding approximation constraints";

  for(idx i = 0; i < dimension; ++i)
  {
    for(const Vertex& current : graph.get_vertices())
    {
      GRBLinExpr sum;

      double fractional_control_sum = 0.;

      for(const Vertex& previous : graph.get_vertices())
      {
        if(current < previous)
        {
          break;
        }

        fractional_control_sum += fractional_controls(previous).at(i);

        sum += variables(previous).at(i);
      }

      {
        std::ostringstream namebuf;

        namebuf << "approx_lower" << current.get_index() << "_" << i;

        model.addConstr((fractional_control_sum - upper_bound) <= sum, namebuf.str());
      }

      {
        std::ostringstream namebuf;

        namebuf << "approx_upper" << current.get_index() << "_" << i;

        model.addConstr(sum <= (upper_bound + fractional_control_sum), namebuf.str());
      }
    }
  }
}

void DAGProgram::add_objective()
{
  Log(debug) << "Adding objective variables";

  auto add_objective_term = [&](const GRBVar& first,
                                const GRBVar& second,
                                bool first_flipped,
                                bool second_flipped,
                                double cost) -> GRBVar
    {
      GRBLinExpr first_expr, second_expr;

      if(first_flipped)
      {
        first_expr = 1. - first;
      }
      else
      {
        first_expr = first;
      }

      if(second_flipped)
      {
        second_expr = 1. - second;
      }
      else
      {
        second_expr = second;
      }

      GRBVar var = model.addVar(0.,
                                1.,
                                cost,
                                GRB_CONTINUOUS,
                                "");

      model.addConstr(first_expr + second_expr <= 1 + var);

      return var;
    };

  for(const Edge& edge : graph.get_edges())
  {
    Vertex source = edge.get_source();
    Vertex target = edge.get_target();

    for(idx i = 0; i < dimension; ++i)
    {
      GRBVar source_var = variables(source).at(i);
      GRBVar target_var = variables(target).at(i);

      for(auto source_val : {0, 1})
      {
        for(auto target_val : {0, 1})
        {
          const double edge_cost = costs(edge,
                                         source_val,
                                         target_val);

          if(edge_cost == 0.)
          {
            continue;
          }

          assert(edge_cost >= 0.);

          bool source_inv = source_val == 0;
          bool target_inv = target_val == 0;

          GRBVar var = add_objective_term(source_var,
                                          target_var,
                                          source_inv,
                                          target_inv,
                                          edge_cost);

          cost_variables(edge).push_back(CostVariable{
              i,
              source_inv,
              target_inv,
              var});

        }
      }
    }
  }
}

VertexMap<Controls> DAGProgram::solve()
{
  Log(info) << "Solving model";

  model.optimize();

  VertexMap<Controls> rounded_controls(graph, Controls(dimension, 0.));

  const double eps = 1e-5;

  for(const Vertex& vertex : graph.get_vertices())
  {
    for(idx i = 0; i < dimension; ++i)
    {
      const double var_value = variables(vertex).at(i).get(GRB_DoubleAttr_X);

      assert(cmp::integral(var_value, eps));

      rounded_controls(vertex).at(i) = cmp::round(var_value, eps);
    }
  }

  assert(controls_are_integral(graph, rounded_controls));
  assert(controls_are_convex(graph, rounded_controls));
  assert(cmp::le(control_distance(graph, fractional_controls, rounded_controls),
                 upper_bound));

  double obj_val = model.get(GRB_DoubleAttr_ObjVal);

  double control_cost = costs.evaluate(graph, rounded_controls);

  assert(cmp::eq(control_cost, obj_val, eps));

  return rounded_controls;
}

void DAGProgram::set_initial_solution(const VertexMap<Controls>& initial_solution)
{
  for(const Vertex& vertex : graph.get_vertices())
  {
    for(idx i = 0; i < dimension; ++i)
    {
      variables(vertex).at(i).set(GRB_DoubleAttr_Start,
                                  initial_solution(vertex).at(i));
    }
  }

  for(const Edge& edge : graph.get_edges())
  {
    Vertex source = edge.get_source();
    Vertex target = edge.get_target();

    for(auto& cost_variable : cost_variables(edge))
    {
      const idx i = cost_variable.i;

      const double source_val = initial_solution(source).at(i);
      const double target_val = initial_solution(target).at(i);

      const bool source_inv = (source_val == 0.);
      const bool target_inv = (target_val == 0.);

      if(source_inv == cost_variable.source_inv &&
         target_inv == cost_variable.target_inv)
      {
        cost_variable.var.set(GRB_DoubleAttr_Start, 1.);
      }
      else
      {
        cost_variable.var.set(GRB_DoubleAttr_Start, 0.);
      }
    }
  }
}
