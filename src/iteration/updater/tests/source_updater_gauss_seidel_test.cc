#include "iteration/updater/source_updater_gauss_seidel.h"

#include <memory>

#include <deal.II/base/mpi.h>
#include <deal.II/lac/petsc_parallel_vector.h>

#include "test_helpers/test_helper_functions.h"
#include "test_helpers/gmock_wrapper.h"
#include "data/moment_types.h"
#include "data/system.h"
#include "data/system/system_types.h"
#include "data/system/tests/right_hand_side_mock.h"
#include "formulation/tests/cfem_stamper_mock.h"
#include "formulation/cfem_stamper_i.h"
#include "test_helpers/test_assertions.h"


namespace  {

using namespace bart;
using data::system::MPIVector;

using ::testing::An;
using ::testing::Ref;
using ::testing::DoDefault;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::WithArgs;
using ::testing::_;

/* This test class verifies the operation of the SourceUpdaterGaussSeidel class.
 * It uses the template of it that uses a CFEMStamperI, a mock version of
 * that object will be used. The required System object will be explicitly
 * created, because it is just a struct, but some of the stored objects
 * (the right hand side) will also use mocks.
 */
class IterationSourceUpdaterGaussSeidelTest : public ::testing::Test {
 protected:
  using CFEMSourceUpdater = iteration::updater::SourceUpdaterGaussSeidel<formulation::CFEMStamperI>;
  using VariableTerms = data::system::RightHandSideI::VariableTerms;

  // Required objects
  // Test system
  data::System test_system_;
  // Vector returned by the mock RightHandSide object when the variable
  // right hand side vector is requested. This is what should be stamped.
  std::shared_ptr<MPIVector> source_vector_ptr_;
  MPIVector expected_vector_;

  // Required mocks
  std::unique_ptr<formulation::CFEM_StamperMock> mock_stamper_ptr_;
  std::unique_ptr<data::system::RightHandSideMock> mock_rhs_ptr_;

  void SetUp() override;
};

/* Sets up the tests. This function first creates the mock objects to be used
 * by the testing, then establishes any default behaviors for NiceMocks. The
 * right hand side vector can be handed off to the System object, but the
 * stamper needs to be given to the test updater _in_ the tests because, as a
 * required dependency, it is passed in the constructor.
 */
void IterationSourceUpdaterGaussSeidelTest::SetUp() {
  mock_stamper_ptr_ = std::make_unique<formulation::CFEM_StamperMock>();
  mock_rhs_ptr_ = std::make_unique<NiceMock<data::system::RightHandSideMock>>();

  ON_CALL(*mock_rhs_ptr_, GetVariablePtr(An<data::system::Index>(),_))
      .WillByDefault(Return(source_vector_ptr_));
  ON_CALL(*mock_rhs_ptr_, GetVariablePtr(An<data::system::GroupNumber>(),_))
      .WillByDefault(Return(source_vector_ptr_));

  /* Create and populate moment maps. The inserted MomentVectors can be empty
   * because we will check that the correct ones are passed by reference not
   * entries.
   */
  data::system::MomentsMap current_iteration, previous_iteration;

  for (data::system::GroupNumber group = 0; group < 5; ++group) {
    for (data::system::HarmonicL l = 0; l < 2; ++l) {
      for (data::system::HarmonicM m = -l; m <= l; ++m) {
        data::system::MomentVector current_moment, previous_moment;
        current_iteration[{group, l, m}] = current_moment;
        previous_iteration[{group, l, m}] = previous_moment;
      }
    }
  }

  /* Initialize MPI Vectors */
  source_vector_ptr_ = std::make_shared<MPIVector>();
  auto n_processes = dealii::Utilities::MPI::n_mpi_processes(MPI_COMM_WORLD);
  source_vector_ptr_->reinit(MPI_COMM_WORLD,
                             n_processes*5,
                             5);
  expected_vector_.reinit(*source_vector_ptr_);
}
// Fills an MPI vector with value
void StampMPIVector(MPIVector &to_fill, double value = 2) {
  auto [local_begin, local_end] = to_fill.local_range();
  for (unsigned int i = local_begin; i < local_end; ++i)
    to_fill(i) += value;
  to_fill.compress(dealii::VectorOperation::add);
}

// Verifies that the Updater takes ownership of the stamper.
TEST_F(IterationSourceUpdaterGaussSeidelTest, Constructor) {
  CFEMSourceUpdater test_updater(std::move(mock_stamper_ptr_));
  test_system_.right_hand_side_ptr_ = std::move(mock_rhs_ptr_);

  EXPECT_EQ(mock_stamper_ptr_, nullptr);
}

// Verifies operation of the UpdateScatteringSource function
TEST_F(IterationSourceUpdaterGaussSeidelTest, UpdateScatteringSourceTest) {
  data::system::GroupNumber group = btest::RandomDouble(0, 6);
  data::system::AngleIndex angle = btest::RandomDouble(0, 10);
  data::system::Index index = {group, angle};
  // Fill source vector with the value 2
  StampMPIVector(*source_vector_ptr_, 3);
  StampMPIVector(expected_vector_, group + 3);


  /* Call expectations, expect to retrieve the scattering term vector from RHS
   * and then stamp it. We invoke the StampMPIVector function, which STAMPS a
   * vector. We make sure that the original value of 3, filled above, was zerod
   * out and replace by the default call value of 2.
   */
  EXPECT_CALL(*mock_rhs_ptr_, GetVariablePtr(index,
                                             VariableTerms::kScatteringSource))
      .WillOnce(DoDefault());
  EXPECT_CALL(*mock_stamper_ptr_,
      StampScatteringSource(Ref(*source_vector_ptr_), // Vector to stamp from mock RHS
                            group,                    // Group specified by the test
                            Ref(test_system_.current_iteration[{group, 0, 0}]), // Current scalar flux for in-group
                            Ref(test_system_.current_iteration)))               // Current moments for out-group
      .WillOnce(WithArgs<0,1>(Invoke(StampMPIVector)));


  test_system_.right_hand_side_ptr_ = std::move(mock_rhs_ptr_);
  CFEMSourceUpdater test_updater(std::move(mock_stamper_ptr_));
  test_updater.UpdateScatteringSource(test_system_, group, angle);

  EXPECT_TRUE(bart::testing::CompareMPIVectors(*source_vector_ptr_,
                                               expected_vector_));

}
} // namespace