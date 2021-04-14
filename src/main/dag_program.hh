#ifndef DAG_PROGRAM_HH
#define DAG_PROGRAM_HH

#include <gurobi_c++.h>

#include "controls.hh"
#include "cost_function.hh"
#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "graph/vertex_map.hh"

class DAGProgram
{
private:
  GRBEnv env;
  GRBModel model;

  const Graph& graph;
  const VertexMap<Controls>& fractional_controls;
  const CostFunction& costs;

  VertexMap<std::vector<GRBVar>> variables;

  void create_variables();
  void add_vertex_constraints();
  void add_objective();
  void add_approximation_constraints();

  const Vertex source;
  const idx dimension;
  const double upper_bound;

  struct CostVariable
  {
    idx i;
    bool source_inv;
    bool target_inv;
    GRBVar var;
  };

  EdgeMap<std::vector<CostVariable>> cost_variables;

public:
  DAGProgram(const Graph& graph,
             const VertexMap<Controls>& fractional_controls,
             const CostFunction& costs);

  VertexMap<Controls> solve();

  void set_initial_solution(const VertexMap<Controls>& initial_solution);
};


#endif /* DAG_PROGRAM_HH */
