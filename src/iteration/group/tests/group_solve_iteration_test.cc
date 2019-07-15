#include "iteration/group/group_solve_iteration.h"

#include <memory>

#include "convergence/tests/final_checker_mock.h"
#include "solver/group/tests/single_group_solver_mock.h"
#include "test_helpers/gmock_wrapper.h"

namespace  {

using namespace bart;

template <typename DimensionWrapper>
class IterationGroupSolveIterationTest : public ::testing::Test {
 public:
  static constexpr int dim = DimensionWrapper::value;

  using TestGroupIterator = iteration::group::GroupSolveIteration;
  using GroupSolver = solver::group::SingleGroupSolverMock;
  using ConvergenceChecker = convergence::FinalCheckerMock<system::moments::MomentVector>;

  // Test object
  std::unique_ptr<TestGroupIterator> test_iterator_ptr_;

  // Mock objects
  std::unique_ptr<GroupSolver> single_group_solver_ptr_;
  std::unique_ptr<ConvergenceChecker> convergence_checker_ptr_;

  // Observing pointers
  GroupSolver* single_group_obs_ptr_ = nullptr;
  ConvergenceChecker* convergence_checker_obs_ptr_ = nullptr;

  void SetUp() override;
};

TYPED_TEST_CASE(IterationGroupSolveIterationTest, bart::testing::AllDimensions);

template <typename DimensionWrapper>
void IterationGroupSolveIterationTest<DimensionWrapper>::SetUp() {
  single_group_solver_ptr_ = std::make_unique<GroupSolver>();
  single_group_obs_ptr_ = single_group_solver_ptr_.get();
  convergence_checker_ptr_ = std::make_unique<ConvergenceChecker>();
  convergence_checker_obs_ptr_ = convergence_checker_ptr_.get();

  test_iterator_ptr_ = std::make_unique<TestGroupIterator>(
      std::move(single_group_solver_ptr_),
      std::move(convergence_checker_ptr_)
      );
}

TYPED_TEST(IterationGroupSolveIterationTest, Constructor) {
  using GroupSolver = solver::group::SingleGroupSolverMock;
  using ConvergenceChecker = convergence::FinalCheckerMock<system::moments::MomentVector>;

  auto single_group_test_ptr = dynamic_cast<GroupSolver*>(
      this->test_iterator_ptr_->group_solver());
  auto convergence_checker_test_ptr = dynamic_cast<ConvergenceChecker*>(
      this->test_iterator_ptr_->convergence_checker_ptr());

  EXPECT_NE(nullptr, single_group_test_ptr);
  EXPECT_NE(nullptr, convergence_checker_test_ptr);
}

} // namespace