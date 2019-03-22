#include "convergence/final_checker_or_n.h"

#include <memory>
#include <optional>

#include <gtest/gtest.h>

#include "convergence/status.h"
#include "convergence/moments/tests/single_moment_checker_mock.h"
#include "data/moment_types.h"
#include "test_helpers/gmock_wrapper.h"

namespace {

using ::testing::_;
using ::testing::Return;
using namespace bart::convergence;

class ConvergenceFinalCheckerOrNSingleMomentTest : public ::testing::Test {
 protected:
  using FinalSingleMomentChecker =
      FinalCheckerOrN<bart::data::MomentVector, moments::SingleMomentCheckerI>;
  std::unique_ptr<moments::SingleMomentCheckerMock> checker_ptr;
  bart::data::MomentVector moment_one, moment_two;
  void SetUp() override;
};

void ConvergenceFinalCheckerOrNSingleMomentTest::SetUp() {
  checker_ptr = std::make_unique<moments::SingleMomentCheckerMock>();
  ON_CALL(*checker_ptr, CheckIfConverged(_,_)).
      WillByDefault(Return(true));
}

// -- CUSTOM ASSERTION FOR COMPARING STATUS STRUCTS --

::testing::AssertionResult CompareStatus(const Status lhs, const Status rhs) {
  if (lhs.iteration_number != rhs.iteration_number) {
    return ::testing::AssertionFailure() << "Iteration number mismatch "
                                         << lhs.iteration_number << " != "
                                         << rhs.iteration_number;
  } else if (lhs.max_iterations != rhs.max_iterations) {
    return ::testing::AssertionFailure() << "Max iteration number mismatch "
                                         << lhs.max_iterations << " != "
                                         << rhs.max_iterations;
  } else if (lhs.is_complete != rhs.is_complete) {
    return ::testing::AssertionFailure() << "Convergence completion mismatch "
                                         << lhs.is_complete << " != "
                                         << rhs.is_complete;
  } else if (lhs.failed_index != rhs.failed_index) {
    return ::testing::AssertionFailure() << "Failed index mismatch "
                                         << lhs.failed_index.value_or(-1) << " != "
                                         << rhs.failed_index.value_or(-1);
  } else if (lhs.delta != rhs.delta) {
    return ::testing::AssertionFailure() << "Delta mismatch "
                                         << lhs.failed_index.value_or(-1) << " != "
                                         << rhs.failed_index.value_or(-1);
  }
  return ::testing::AssertionSuccess();
}

TEST_F(ConvergenceFinalCheckerOrNSingleMomentTest, Constructor) {
  FinalSingleMomentChecker test_checker(std::move(checker_ptr));

  EXPECT_EQ(checker_ptr, nullptr);
}

// Verify setters and getters work properly
TEST_F(ConvergenceFinalCheckerOrNSingleMomentTest, SettersAndGetters) {
  FinalSingleMomentChecker test_checker(std::move(checker_ptr));

  EXPECT_EQ(test_checker.max_iterations(), 100);
  test_checker.SetMaxIterations(50);
  EXPECT_EQ(test_checker.max_iterations(), 50);

  EXPECT_EQ(test_checker.iteration(), 0);
  test_checker.SetIteration(10);
  EXPECT_EQ(test_checker.iteration(), 10);
}

// -- CONVERGENCE TESTS --

TEST_F(ConvergenceFinalCheckerOrNSingleMomentTest, GoodConvergence) {
  FinalSingleMomentChecker test_checker(std::move(checker_ptr));
  Status good_convergence = {1, 100, true, std::nullopt, std::nullopt};
  auto result = test_checker.CheckFinalConvergence(moment_one, moment_two);
  EXPECT_TRUE(CompareStatus(result, good_convergence));
}

// -- ERRORS --
// Verify setters and getters throw the correct errors
TEST_F(ConvergenceFinalCheckerOrNSingleMomentTest, SettersBadValues) {
  FinalSingleMomentChecker test_checker(std::move(checker_ptr));

  EXPECT_ANY_THROW(test_checker.SetIteration(-10));
  EXPECT_ANY_THROW(test_checker.SetMaxIterations(-10));
  EXPECT_ANY_THROW(test_checker.SetMaxIterations(0));

}


} // namespace




