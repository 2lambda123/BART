#include "quadrature/calculators/spherical_harmonic_zeroth_moment.h"

#include <memory>

#include <deal.II/base/mpi.h>
#include <deal.II/lac/petsc_parallel_vector.h>

#include "system/system_types.h"
#include "system/moments/spherical_harmonic_types.h"
#include "quadrature/angular/tests/angular_quadrature_set_mock.h"
#include "system/solution/tests/mpi_angular_mock.h"
#include "test_helpers/gmock_wrapper.h"

namespace {

using namespace bart;

using ::testing::Ref;

void SetVector(system::MPIVector& to_set, double value) {
  auto [first_row, last_row] = to_set.local_range();
  for (unsigned int i = first_row; i < last_row; ++i)
    to_set[i] = value;
}

template <typename DimensionWrapper>
class QuadCalcSphericalHarmonicMomentsOnlyScalar : public ::testing::Test {
 protected:
  static constexpr int dim = DimensionWrapper::value;
  // Aliases
  using AngularQuadratureSetType = quadrature::angular::AngularQuadratureSetMock<dim>;
  using MomentCalculatorType = quadrature::calculators::SphericalHarmonicZerothMoment<dim>;

  // Pointer to tested object
  std::unique_ptr<MomentCalculatorType> test_calculator;

  // Supporting objects
  system::solution::MPIAngularMock mock_solution_;
  std::array<system::MPIVector, 3> mpi_vectors_;

  // Test object dependency
  std::shared_ptr<AngularQuadratureSetType> mock_angular_quad_;

  // Observing pointers
  AngularQuadratureSetType* angular_quad_obs_ptr_;

  // Test parameters
  const int n_processes = dealii::Utilities::MPI::n_mpi_processes(MPI_COMM_WORLD);
  const int n_entries_per_proc = 10;

  void SetUp() override;
};

template <typename DimensionWrapper>
void QuadCalcSphericalHarmonicMomentsOnlyScalar<DimensionWrapper>::SetUp() {

  // Instantiate mock objects
  mock_angular_quad_ = std::make_shared<AngularQuadratureSetType>();

  // Instantiate object to be tested
  test_calculator = std::make_unique<MomentCalculatorType>(mock_angular_quad_);

  // Set up observation pointers
  angular_quad_obs_ptr_ = dynamic_cast<AngularQuadratureSetType*>(
      test_calculator->angular_quadrature_set_ptr());


  for (auto& mpi_vector : mpi_vectors_) {
    mpi_vector.reinit(MPI_COMM_WORLD,
                      n_processes * n_entries_per_proc,
                      n_entries_per_proc);
  };

  SetVector(mpi_vectors_[0], 1);
  SetVector(mpi_vectors_[1], 10);
  SetVector(mpi_vectors_[2], 100);
}

TYPED_TEST_CASE(QuadCalcSphericalHarmonicMomentsOnlyScalar,
                bart::testing::AllDimensions);

TYPED_TEST(QuadCalcSphericalHarmonicMomentsOnlyScalar, Constructor) {
  auto& mock_angular_quad_ = this->mock_angular_quad_;
  auto& angular_quad_obs_ptr_ = this->angular_quad_obs_ptr_;

  EXPECT_EQ(mock_angular_quad_.use_count(), 2);
  EXPECT_THAT(*mock_angular_quad_, Ref(*angular_quad_obs_ptr_));
}


TYPED_TEST(QuadCalcSphericalHarmonicMomentsOnlyScalar, CalculateMomentsMPI) {
  const int group = 0;
  const int total_angles = 3;

  for (int angle = 0; angle < total_angles; ++angle) {

  }
}


} // namespace
