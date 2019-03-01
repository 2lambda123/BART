#include "convergence/flux/multi_checker_sequential.h"

#include <memory>
#include <optional>

#include <deal.II/lac/petsc_parallel_vector.h>

#include "test_helpers/test_helper_functions.h"
#include "test_helpers/gmock_wrapper.h"
#include "convergence/flux/tests/single_checker_mock.h"

using ::testing::_;

class MultiCheckerSequentialTest : public ::testing::Test {
 protected:
  std::unique_ptr<bart::convergence::flux::SingleCheckerMock> checker_mock;

  bart::data::MultiFluxPtrs current;
  bart::data::MultiFluxPtrs previous;

  void SetUp() override;
  void FillGroupFluxes(bart::data::MultiFluxPtrs &to_fill, int n_ptrs);

};
/* Fills a MultiFluxPtrs object with n_ptrs random flux pointers */
void MultiCheckerSequentialTest::FillGroupFluxes(
    bart::data::MultiFluxPtrs &to_fill, int n_ptrs) {
  
  for (int i = 0; i < n_ptrs; ++i) {
    auto flux = std::make_unique<bart::data::Flux>();    
    flux->reinit(MPI_COMM_WORLD, 5, 5);
    
    auto random_vector = btest::RandomVector(5, 0, 2);
    
    for (unsigned int j = 0; j < flux->size(); ++j)
      (*flux)(j) = random_vector[j];
    
    flux->compress(dealii::VectorOperation::values::insert);
    to_fill[i] = std::move(flux);
  }
}

void MultiCheckerSequentialTest::SetUp() {
  // Make mock flux convergence checker
  checker_mock = std::make_unique<bart::convergence::flux::SingleCheckerMock>();
}


TEST_F(MultiCheckerSequentialTest, Constructor) {
  bart::convergence::flux::MultiCheckerSequential
      sequential_checker(std::move(checker_mock));
  // Verify constructor took ownership of the checker pointer
  EXPECT_EQ(checker_mock, nullptr);
}

TEST_F(MultiCheckerSequentialTest, EmptyGroups) {
  bart::convergence::flux::MultiCheckerSequential
      sequential_checker(std::move(checker_mock));
  // Verify error thrown if empty fluxes provided
  EXPECT_ANY_THROW(sequential_checker.CheckIfConverged(current, previous));
}

TEST_F(MultiCheckerSequentialTest, DifferentGroupSizes) {
  bart::convergence::flux::MultiCheckerSequential
      sequential_checker(std::move(checker_mock));
  // Verify error thrown if fluxes have different numbers of flux pointers
  FillGroupFluxes(current, 5);
  FillGroupFluxes(previous, 4);
  EXPECT_ANY_THROW(sequential_checker.CheckIfConverged(current, previous));
  EXPECT_FALSE(sequential_checker.is_converged());
  EXPECT_FALSE(sequential_checker.failed_index().has_value());
  EXPECT_FALSE(sequential_checker.delta().has_value());
}

TEST_F(MultiCheckerSequentialTest, GoodMatch) {

  // Verify good match
  EXPECT_CALL(*checker_mock, CheckIfConverged(_,_)).
      WillRepeatedly(::testing::Return(true));
  EXPECT_CALL(*checker_mock, delta()).
      WillOnce(::testing::Return(std::make_optional(0.123)));

  bart::convergence::flux::MultiCheckerSequential
      sequential_checker(std::move(checker_mock));
  
  FillGroupFluxes(current, 3);
  FillGroupFluxes(previous, 3);

  EXPECT_TRUE(sequential_checker.CheckIfConverged(current, previous));
  EXPECT_TRUE(sequential_checker.is_converged());
  EXPECT_TRUE(sequential_checker.delta().has_value());
  EXPECT_FALSE(sequential_checker.failed_index().has_value());
  EXPECT_DOUBLE_EQ(sequential_checker.delta().value(), 0.123);
}

TEST_F(MultiCheckerSequentialTest, BadGroupMatch) {
  // Verify error if group values are not the same
  EXPECT_CALL(*checker_mock, CheckIfConverged(_,_)).
      WillRepeatedly(::testing::Return(true));

  bart::convergence::flux::MultiCheckerSequential
      sequential_checker(std::move(checker_mock));
  
  FillGroupFluxes(current, 3);
  FillGroupFluxes(previous, 3);

  auto flux = current.extract(0);
  flux.key() = 6;
  current.insert(std::move(flux));

  EXPECT_ANY_THROW(sequential_checker.CheckIfConverged(current, previous));
  EXPECT_FALSE(sequential_checker.is_converged());
  EXPECT_FALSE(sequential_checker.failed_index().has_value());
  EXPECT_FALSE(sequential_checker.delta().has_value());
}

TEST_F(MultiCheckerSequentialTest, BadMatch) {
  // Check if correct action on failed convergence 
  EXPECT_CALL(*checker_mock, CheckIfConverged(_,_)).
      WillOnce(::testing::Return(true)).
      WillOnce(::testing::Return(false));

  EXPECT_CALL(*checker_mock, delta()).
      WillOnce(::testing::Return(std::make_optional(0.123)));

  bart::convergence::flux::MultiCheckerSequential
      sequential_checker(std::move(checker_mock));
  
  FillGroupFluxes(current, 3);
  FillGroupFluxes(previous, 3);

  EXPECT_FALSE(sequential_checker.CheckIfConverged(current, previous));
  EXPECT_FALSE(sequential_checker.is_converged());
  EXPECT_TRUE(sequential_checker.failed_index().has_value());
  EXPECT_EQ(sequential_checker.failed_index().value(), 1);
  EXPECT_TRUE(sequential_checker.delta().has_value());
  EXPECT_EQ(sequential_checker.delta().value(), 0.123);
}
