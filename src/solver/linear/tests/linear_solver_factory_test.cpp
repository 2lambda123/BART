#include "solver/linear/factory.hpp"

#include "solver/linear/gmres.h"
#include "test_helpers/gmock_wrapper.h"
#include "test_helpers/test_helper_functions.h"

namespace  {

namespace solver = bart::solver;
namespace test_helpers = bart::test_helpers;

TEST(SolverFactoryTest, GMRES) {
  using ExpectedType = solver::linear::GMRES;
  using SolverName = solver::linear::LinearSolverName;
  const int max_iterations{test_helpers::RandomInt(200, 1000)};
  const double tolerance{test_helpers::RandomDouble(1e-16, 1e-10)};
  auto gmres_ptr = solver::linear::LinearIFactory<int, double>::get()
      .GetConstructor(SolverName::kGMRES)(max_iterations, tolerance);
  ASSERT_NE(gmres_ptr, nullptr);
  auto dynamic_ptr = dynamic_cast<ExpectedType*>(gmres_ptr.get());
  ASSERT_NE(dynamic_ptr, nullptr);
  EXPECT_EQ(dynamic_ptr->max_iterations(), max_iterations);
  EXPECT_EQ(dynamic_ptr->convergence_tolerance(), tolerance);
}

} // namespace
