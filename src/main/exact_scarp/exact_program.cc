#include "exact_program.hh"

#include <algorithm>
#include <sstream>

#include "cmp.hh"
#include "log.hh"
#include "graph/vertex_map.hh"

VertexMap<idx> get_prefix_map(const Graph& graph)
{
  VertexMap<idx> prefix_map(graph, 0);

  for(const Vertex& vertex : graph.get_vertices())
  {
    const idx vertex_index = vertex.get_index();

    for(const Edge& incoming : graph.get_incoming(vertex))
    {
      const idx source_index = incoming.get_source().get_index();

      assert(vertex_index > source_index);

      prefix_map(vertex) = std::max(prefix_map(vertex),
                                    vertex_index -source_index);
    }
  }

  auto rit = graph.get_vertices().rbegin();
  auto rend = graph.get_vertices().rend();

  for(; rit != rend; ++rit)
  {
    Vertex vertex = *rit;

    idx prefix_length = prefix_map(vertex);

    if(prefix_length == 0)
    {
      prefix_map(vertex) = 1;
      continue;
    }

    auto fit = rit;

    ++fit;
    --prefix_length;

    for(;prefix_length > 0 && fit != rend;
        ++fit, --prefix_length)
    {
      Vertex vertex = *fit;

      if(prefix_length <= prefix_map(vertex))
      {
        break;
      }

      prefix_map(vertex) = prefix_length;
    }
  }

  return prefix_map;
}

ExactProgram::ExactProgram(const Graph& graph,
                           const CostFunction& costs,
                           const VertexMap<Controls>& fractional_controls,
                           bool vanishing_constraints)
  : graph(graph),
    prefix_map(get_prefix_map(graph)),
    costs(costs),
    fractional_controls(fractional_controls),
    vanishing_constraints(vanishing_constraints),
    source(*graph.get_vertices().begin()),
    dimension(fractional_controls(source).size()),
    upper_bound(max_control_deviation(dimension)),
    labels(graph, LabelFront(dimension, LabelSet())),
    fractional_control_sums(dimension, 0.),
    num_labels(0)
{
  assert(controls_are_convex(graph, fractional_controls));
}

VertexMap<Controls>
ExactProgram::get_controls(ExactLabelPtr label) const
{
  VertexMap<Controls> controls(graph, Controls(dimension, 0.));

  while(label)
  {
    controls(label->get_vertex()).at(label->get_current_control()) = 1.;
    label = label->get_predecessor();
  }

  return controls;
}

void ExactProgram::clear()
{
  num_labels = 0;

  for(idx i = 0; i < dimension; ++i)
  {
    fractional_control_sums.at(i) = 0.;
  }

  for(const Vertex& vertex : graph.get_vertices())
  {
    for(idx i = 0; i < dimension; ++i)
    {
      labels(vertex).at(i).clear();
    }
  }
}

void ExactProgram::create_initial_labels()
{
  for(idx i = 0; i < dimension; ++i)
  {
    const double fractional_control = fractional_controls(source).at(i);

    if(vanishing_constraints && cmp::zero(fractional_control))
    {
      continue;
    }

    if(std::abs(1. - fractional_control) > upper_bound)
    {
      continue;
    }

    labels(source).at(i).insert(std::make_shared<ExactLabel>(i, source, dimension, prefix_map(source)));
  }
}

void ExactProgram::expand(Vertex source, Vertex target)
{
  Controls previous_controls(dimension, 0.);
  Controls next_controls(dimension, 0.);

  VertexMap<std::optional<Edge>> predecessors(graph, {});

  idx num_predecessors = 0;

  for(const Edge& incoming : graph.get_incoming(target))
  {
    predecessors(incoming.get_source()) = incoming;
    ++num_predecessors;
  }

  for(idx i = 0; i < dimension; ++i)
  {
    for(const auto& label : labels(source).at(i))
    {
      assert(label->get_current_control() == i);

      auto& target_labels = labels(target);

      for(idx j = 0; j < dimension; ++j)
      {
        // feasibility test
        {
          if(vanishing_constraints && cmp::zero(fractional_controls(target).at(j)))
          {
            continue;
          }

          bool feasible = true;

          for(idx k = 0; k < dimension; ++k)
          {
            const double control_sum = label->get_control_sums().at(k) + (j == k);
            const double fractional_control_sum = fractional_control_sums.at(k);

            if(std::abs(control_sum - fractional_control_sum) > upper_bound)
            {
              feasible = false;
              break;
            }
          }

          if(!feasible)
          {
            if(debugging_enabled())
            {
              auto next_label = std::make_shared<ExactLabel>(j,
                                                             target,
                                                             0.,
                                                             prefix_map(target),
                                                             label);

              double label_dist = label_distance(next_label);

              assert(label_dist > upper_bound);
            }

            continue;
          }
        }

        next_controls.at(j) = 1.;

        double additional_cost = 0.;

        // cost computation
        {
          idx num_remaining_predecessors = num_predecessors;

          for(auto predecessor_label = label;
              num_remaining_predecessors > 0;
              predecessor_label = predecessor_label->get_predecessor())
          {
            assert(predecessor_label);

            auto edge = predecessors(predecessor_label->get_vertex());

            if(edge)
            {
              previous_controls.at(predecessor_label->get_current_control()) = 1;

              const double edge_cost = costs(*edge,
                                             previous_controls,
                                             next_controls);

              assert(edge_cost >= 0.);

              additional_cost += edge_cost;

              --num_remaining_predecessors;

              previous_controls.at(predecessor_label->get_current_control()) = 0;
            }
          }

          assert(num_remaining_predecessors == 0);
        }

        // label insertions
        {
          auto next_label = std::make_shared<ExactLabel>(j,
                                                         target,
                                                         label->get_cost() + additional_cost,
                                                         prefix_map(target),
                                                         label);

          auto it = target_labels.at(j).find(next_label);

          if(it == target_labels.at(j).end())
          {
            target_labels.at(j).insert(next_label);

            ++num_labels;
          }
          else
          {
            assert((*it)->get_control_sums() == next_label->get_control_sums());
            assert((*it)->get_prefix() == next_label->get_prefix());

            if((*it)->get_cost() > next_label->get_cost())
            {
              (*it)->replace(*next_label);
            }
          }

          if(debugging_enabled())
          {
            const double actual_cost = get_costs(next_label);

            assert(cmp::eq(actual_cost, next_label->get_cost()));

            std::vector<idx> actual_control_sums = get_control_sums(next_label);

            assert(actual_control_sums == next_label->get_control_sums());
          }
        }

        next_controls.at(j) = 0.;
      }
    }
  }
}

void ExactProgram::expand_all()
{
  auto source_it = graph.get_vertices().begin();
  auto target_it = graph.get_vertices().begin();
  auto end_it = graph.get_vertices().end();

  ++target_it;

  auto add_fractional_controls = [&](Vertex vertex)
    {
      for(idx i = 0; i < dimension; ++i)
      {
        fractional_control_sums.at(i) += fractional_controls(vertex).at(i);
      }
    };

  add_fractional_controls(source);

  for(; target_it != end_it; ++source_it, ++target_it)
  {
    Vertex source = *source_it;
    Vertex target = *target_it;

    assert(source < target);

    if(debugging_enabled())
    {
      for(idx i = 0; i < dimension; ++i)
      {
        for(const auto label : labels(source).at(i))
        {
          double label_dist = label_distance(label);

          assert(label_dist <= upper_bound);
        }
      }

    }

    add_fractional_controls(target);

    expand(source, target);

    labels(source).clear();
  }
}

double ExactProgram::get_costs(ExactLabelPtr label)
{
  assert(label);

  ExactLabelPtr current = label;

  VertexMap<int> vertex_controls(graph, -1);

  while(current)
  {
    vertex_controls(current->get_vertex()) = current->get_current_control();
    current = current->get_predecessor();
  }

  double total_cost = 0.;

  Controls source_controls(dimension, 0.);
  Controls target_controls(dimension, 0.);

  for(const Edge& edge : graph.get_edges())
  {
    int source_control = vertex_controls(edge.get_source());
    int target_control = vertex_controls(edge.get_target());

    if(source_control == -1 || target_control == -1)
    {
      continue;
    }

    source_controls.at(source_control) = 1.;
    target_controls.at(target_control) = 1.;

    total_cost += costs(edge,
                        source_controls,
                        target_controls);

    source_controls.at(source_control) = 0.;
    target_controls.at(target_control) = 0.;
  }

  return total_cost;
}

std::vector<idx> ExactProgram::get_control_sums(ExactLabelPtr label)
{
  std::vector<idx> control_sums(dimension, 0);

  ExactLabelPtr current = label;

  while(current)
  {
    control_sums.at(current->get_current_control())++;
    current = current->get_predecessor();
  }

  return control_sums;
}

double ExactProgram::label_distance(ExactLabelPtr label) const
{
  std::vector<idx> control_values;
  std::vector<Vertex> vertices;

  ExactLabelPtr current = label;

  while(current)
  {
    vertices.push_back(current->get_vertex());
    control_values.push_back(current->get_current_control());
    current = current->get_predecessor();
  }

  std::reverse(std::begin(control_values), std::end(control_values));
  std::reverse(std::begin(vertices), std::end(vertices));

  auto vit = std::begin(vertices);
  auto vend = std::end(vertices);

  auto valit = std::begin(control_values);
  auto valend = std::end(control_values);

  std::vector<double> control_sums(dimension, 0.);
  std::vector<double> fractional_sums(dimension, 0.);

  double distance = 0.;

  for(;valit != valend && vit != vend; ++vit, ++valit)
  {
    const Vertex vertex = *vit;
    const idx current_control = *valit;

    // Update control sums
    {
      for(idx i = 0; i < dimension; ++i)
      {
        fractional_sums.at(i) += fractional_controls(vertex).at(i);
      }

      control_sums.at(current_control)++;
    }

    // Update distance
    for(idx i = 0; i < dimension; ++i)
    {
      distance = std::max(std::abs(control_sums.at(i) - fractional_sums.at(i)),
                          distance);
    }
  }

  return distance;
}

VertexMap<Controls> ExactProgram::solve()
{
  clear();

  create_initial_labels();

  expand_all();

  ExactLabelPtr best_label;

  Vertex target = *(graph.get_vertices().rbegin());

  for(idx i = 0; i < dimension; ++i)
  {
    for(const auto& label : labels(target).at(i))
    {
      if(best_label &&
         best_label->get_cost() < label->get_cost())
      {
        continue;
      }
      best_label = label;
    }
  }

  Log(debug) << "Created " << num_labels << " labels";

  auto rounded_controls = get_controls(best_label);

  assert(controls_are_integral(graph, rounded_controls));
  assert(controls_are_convex(graph, rounded_controls));
  assert(cmp::le(control_distance(graph, fractional_controls, rounded_controls),
                 upper_bound));

  assert(cmp::eq(costs.evaluate(graph, rounded_controls),
                 best_label->get_cost()));

  return rounded_controls;
}
