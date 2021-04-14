set(COMMON_SRC
  ansi_color.cc
  cmp.cc
  controls.cc
  control_reader.cc
  control_writer.cc
  cost_function.cc
  dag_program.cc
  grid.cc
  log.cc
  util.cc
  graph/edge.cc
  graph/edge_set.cc
  graph/graph.cc
  graph/vertex.cc
  graph/vertex_set.cc
  scarp/scarp_program.cc
  exact_scarp/exact_label.cc
  exact_scarp/exact_program.cc
  sur/sur.cc)

add_library(common ${COMMON_SRC})

configure_file(defs.hh.in ${CMAKE_BINARY_DIR}/defs.hh)

set(LIBS
  ${Boost_LIBRARIES}
  ${GUROBI_LIBRARIES})

target_link_libraries(common ${LIBS})

add_executable(dag_scarp_solver dag_scarp_solver.cc)
target_link_libraries(dag_scarp_solver common)

add_executable(dag_exact_scarp_solver dag_exact_scarp_solver.cc)
target_link_libraries(dag_exact_scarp_solver common)

add_executable(dag_sur_solver dag_sur_solver.cc)
target_link_libraries(dag_sur_solver common)

add_executable(dag_mip_solver dag_mip_solver.cc)
target_link_libraries(dag_mip_solver common)

add_executable(dag_scarp_batch_solver dag_scarp_batch_solver.cc)
target_link_libraries(dag_scarp_batch_solver common)

add_executable(dag_exact_scarp_batch_solver dag_exact_scarp_batch_solver.cc)
target_link_libraries(dag_exact_scarp_batch_solver common)

add_executable(dag_sur_batch_solver dag_sur_batch_solver.cc)
target_link_libraries(dag_sur_batch_solver common)

add_executable(dag_mip_batch_solver dag_mip_batch_solver.cc)
target_link_libraries(dag_mip_batch_solver common)
