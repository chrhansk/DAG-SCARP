#include "control_reader.hh"

#include <cassert>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "point.hh"
#include "log.hh"

std::vector<std::string> tokenize(const std::string& line)
{
  std::stringstream line_input(line);

  std::vector<std::string> tokens;
  std::string token;

  while(std::getline(line_input, token, ','))
  {
    tokens.push_back(token);
  }

  return tokens;
}

struct Entry
{
  idx i, j;
  std::vector<double> values;
};

std::vector<Entry> read_entries(std::istream& input)
{
  if(!input)
  {
    throw std::invalid_argument("Invalid input");
  }

  std::string line;

  idx line_number = 1;

  idx num_controls = 0;

  bool first_line = true;

  std::vector<Entry> entries;

  while(std::getline(input, line))
  {
    std::vector<std::string> tokens = tokenize(line);

    if(tokens.size() <= 2)
    {
      throw std::invalid_argument("Insufficient number of tokens");
    }

    const double i = std::stod(tokens[0]);
    const double j = std::stod(tokens[1]);

    std::vector<double> controls;

    for(idx k = 2; k < tokens.size(); ++k)
    {
      controls.push_back(std::stod(tokens[k]));
    }

    if(first_line)
    {
      num_controls = controls.size();
    }
    else
    {
      if(num_controls != controls.size())
      {
        throw std::invalid_argument("Invalid number of tokens");
      }
    }

    entries.push_back(Entry{(idx) i, (idx) j, controls});

    ++line_number;
    first_line = false;
  }

  Log(debug) << "Read " << entries.size() << " entries";

  return entries;
}

ReadResult read_file(std::istream& input)
{
  std::vector<Entry> entries = read_entries(input);

  if(entries.size() < 2)
  {
    throw std::invalid_argument("Graph is too small");
  }

  idx m = 0, n = 0;

  for(const auto& entry : entries)
  {
    m = std::max(entry.i, m);
    n = std::max(entry.j, n);
  }

  const idx num_vertices = (m + 1)*(n + 1);

  Graph graph(num_vertices);

  std::unordered_map<Point, Vertex> vertices_by_points;

  VertexMap<Point> points_by_vertices(graph);

  VertexMap<Controls> fractional_controls(graph, {});

  // create first map
  idx p = 0;

  for(const auto& entry : entries)
  {
    Point point(entry.i, entry.j);
    Vertex vertex = graph.get_vertices()[p];

    vertices_by_points.insert({point, vertex});

    points_by_vertices(vertex) = point;

    fractional_controls(vertex) = entry.values;

    ++p;
  }

  auto add_edge = [&](const Vertex& first, const Vertex& second) -> Edge
    {
      if(first < second)
      {
        return graph.add_edge(first, second);
      }
      else
      {
        assert(second < first);

        return graph.add_edge(second, first);
      }
    };

  for(const Vertex& vertex : graph.get_vertices())
  {
    Point point = points_by_vertices(vertex);

    const idx i = point.get_i();
    const idx j = point.get_j();

    if(i > 0)
    {
      Point prev_point(i - 1, j);

      add_edge(vertices_by_points.at(prev_point), vertex);
    }

    if(j > 0)
    {
      Point prev_point(i, j - 1);

      add_edge(vertices_by_points.at(prev_point), vertex);
    }
  }

  if(!controls_are_convex(graph, fractional_controls))
  {
    throw std::invalid_argument("Controls are not convex");
  }

  Log(info) << "Read in a graph with "
            << graph.get_vertices().size()
            << " vertices and "
            << graph.get_edges().size()
            << " edges";

  return ReadResult{graph, fractional_controls, points_by_vertices};
}
