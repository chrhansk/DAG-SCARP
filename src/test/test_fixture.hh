#ifndef TEST_FIXTURE_HH
#define TEST_FIXTURE_HH

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

#include <gtest/gtest.h>

#include "controls.hh"
#include "control_reader.hh"

#include "log.hh"

class TestInstance
{
private:
  const fs::path path;
  const double optimal_objective;

public:
  TestInstance(const fs::path& path,
               double optimal_objective)
    : path(path),
      optimal_objective(optimal_objective)
  {
  }

  const fs::path& get_path() const
  {
    return path;
  }

  const double get_optimal_objective() const
  {
    return optimal_objective;
  }

  ReadResult read() const
  {
    fs::ifstream input{get_path()};

    return read_file(input);
  }

};

std::vector<TestInstance> read_test_instances();


class TestInstances : public ::testing::Test {
protected:
  void SetUp() override
  {
    log_init();

    test_instances = read_test_instances();
  }

  // void TearDown() override {}

  std::vector<TestInstance> test_instances;
};



#endif /* TEST_FIXTURE_HH */
