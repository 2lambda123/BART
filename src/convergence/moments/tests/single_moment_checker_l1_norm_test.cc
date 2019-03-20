#include "convergence/moments/single_moment_checker_l1_norm.h"


#include <gtest/gtest.h>

#include "data/moment_types.h"
#include "test_helpers/test_helper_functions.h"
#include "test_helpers/gmock_wrapper.h"

class SingleMomentCheckerL1NormTest : public ::testing::Test {
 protected:
  bart::convergence::moments::SingleMomentCheckerL1Norm checker{1e-6};
  bart::data::MomentVector moment_one;
  bart::data::MomentVector moment_two;
  void SetUp() override {};
};

TEST_F(SingleMomentCheckerL1NormTest, Dummy) {
  EXPECT_TRUE(true);
}

//void SingleCheckerL1NormTest::SetUp() {
//  flux_one.reinit(MPI_COMM_WORLD, 5, 5);
//  flux_two.reinit(MPI_COMM_WORLD, 5, 5);
//  auto random_vector = btest::RandomVector(5, 0, 2);
//  for (unsigned int i = 0; i < flux_one.size(); ++i) {
//    flux_one(i) = random_vector[i];
//    flux_two(i) = random_vector[i];
//  }
//  flux_one.compress(dealii::VectorOperation::values::insert);
//  flux_two.compress(dealii::VectorOperation::values::insert);
//}
//
//TEST_F(SingleCheckerL1NormTest, SameVector) {
//  EXPECT_TRUE(test_convergence.CheckIfConverged(flux_one, flux_one));
//  EXPECT_TRUE(test_convergence.is_converged());
//}
//
//TEST_F(SingleCheckerL1NormTest, OneThresholdAway) {
//  double to_add = flux_one.l1_norm() * 0.99 * test_convergence.max_delta();
//  flux_two(2) += to_add;
//
//  flux_two.compress(dealii::VectorOperation::values::add);
//  EXPECT_TRUE(test_convergence.CheckIfConverged(flux_one, flux_two));
//  EXPECT_TRUE(test_convergence.CheckIfConverged(flux_two, flux_one));
//  EXPECT_TRUE(test_convergence.is_converged());
//  EXPECT_NEAR(0.99*test_convergence.max_delta(),
//              test_convergence.delta().value(),
//              1e-6);
//}
//
//TEST_F(SingleCheckerL1NormTest, TwoThresholdAway) {
//  double to_add = flux_one.l1_norm() * 2*test_convergence.max_delta();
//  flux_two(2) += to_add;
//  flux_two.compress(dealii::VectorOperation::values::add);
//  EXPECT_FALSE(test_convergence.CheckIfConverged(flux_one, flux_two));
//  EXPECT_FALSE(test_convergence.CheckIfConverged(flux_two, flux_one));
//  EXPECT_FALSE(test_convergence.is_converged());
//  EXPECT_NEAR(2*test_convergence.max_delta(),
//              test_convergence.delta().value(),
//              1e-6);
//}
//
//TEST_F(SingleCheckerL1NormTest, SetMaxDelta) {
//  double to_set = 1e-5;
//  test_convergence.SetMaxDelta(to_set);
//  EXPECT_EQ(test_convergence.max_delta(), to_set);
//
//  double to_add = flux_one.l1_norm() * 0.99 * to_set;
//  flux_two(2) += to_add;
//
//  flux_two.compress(dealii::VectorOperation::values::add);
//  EXPECT_TRUE(test_convergence.CheckIfConverged(flux_one, flux_two));
//  EXPECT_TRUE(test_convergence.CheckIfConverged(flux_two, flux_one));
//
//}
//
