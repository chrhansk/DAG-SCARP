#include "test_fixture.hh"

#include <iostream>

#include <boost/algorithm/string.hpp>

#include "defs.hh"
#include "cmp.hh"

std::vector<TestInstance> read_test_instances()
{
  auto split_string = [](const std::string& line)
    -> std::vector<std::string>
    {
      std::vector<std::string> entries;

      boost::split(entries, line, boost::is_any_of(" "));

      for(auto& entry : entries)
      {
        boost::algorithm::trim(entry);
      }

      return entries;
    };

  auto read_header = [&](const std::string& line)
    -> std::map<std::string, int>
    {
      std::map<std::string, int> fields;

      std::vector<std::string> entries = split_string(line);

      int i = 0;

      for(auto& entry : entries)
      {
        fields.insert({entry, i++});
      }

      return fields;
    };

  fs::path tests_directory{TESTS_DIRECTORY};

  fs::path instance_file = tests_directory / INSTANCE_FILE;

  fs::ifstream input{instance_file};

  std::string line;

  std::getline(input, line);

  auto fields = read_header(line);

  std::vector<TestInstance> instances;

  const int name_field = fields.at("Name");
  const int obj_field = fields.at("Objective");

  while(std::getline(input, line))
  {
    auto row = split_string(line);

    std::string instance_name = row.at(name_field);

    const double objective = std::stof(row.at(obj_field));

    fs::path instance_path = tests_directory / (instance_name + ".csv");

    instances.push_back(TestInstance(instance_path,
                                     objective));
  }

  return instances;
}
