#include "results/output_dealii_vtu.h"

#include <memory>
#include <array>

#include <deal.II/numerics/data_out.h>

#include "domain/tests/definition_mock.h"
#include "system/system.h"
#include "system/moments/tests/spherical_harmonic_mock.h"
#include "test_helpers/gmock_wrapper.h"
#include "test_helpers/dealii_test_domain.h"

namespace {

using namespace bart;

using ::testing::Return, ::testing::ReturnRef;

template <typename DimensionWrapper>
class ResultsOutputDealiiVtuTest
    : public ::testing::Test,
      public ::bart::testing::DealiiTestDomain<DimensionWrapper::value> {
 public:
  static constexpr int dim = DimensionWrapper::value;
  std::shared_ptr<domain::DefinitionI<dim>> domain_ptr_;
  std::unique_ptr<results::OutputDealiiVtu<dim>> test_output_;

  dealii::DataOut<dim> test_data_out_;
  system::System test_system_;

  system::moments::SphericalHarmonicMock* moments_obs_ptr_;

  std::array<double, 2> group_phi_values_{1.45, 1.67};

  const int n_groups = 2;

  void SetUp() override;
};

template <typename DimensionWrapper>
void ResultsOutputDealiiVtuTest<DimensionWrapper>::SetUp() {
  // Dependencies
  domain_ptr_ = std::make_shared<domain::DefinitionMock<dim>>();

  test_output_ = std::make_unique<results::OutputDealiiVtu<dim>>(domain_ptr_);

  // Supporting objects
  auto moments_ptr_ =
      std::make_unique<system::moments::SphericalHarmonicMock>();
  moments_obs_ptr_ = moments_ptr_.get();

  test_system_.current_moments = std::move(moments_ptr_);

  // Set up dealii objects
  this->SetUpDealii();
  test_data_out_.attach_dof_handler(this->dof_handler_);

  for (unsigned int i = 0; i < this->vector_1.size(); ++i) {
    if (this->vector_1.in_local_range(i))
      this->vector_1[i] = group_phi_values_[0];
  }

  for (unsigned int i = 0; i < this->vector_2.size(); ++i) {
    if (this->vector_2.in_local_range(i))
      this->vector_2[i] = group_phi_values_[1];
  }

  test_data_out_.add_data_vector(this->vector_1, "scalar_flux_group_0");
  test_data_out_.add_data_vector(this->vector_2, "scalar_flux_group_1");

  test_data_out_.build_patches();
}

TYPED_TEST_CASE(ResultsOutputDealiiVtuTest, bart::testing::AllDimensions);

TYPED_TEST(ResultsOutputDealiiVtuTest, Constructor) {
  constexpr int dim = this->dim;
  EXPECT_EQ(this->domain_ptr_.use_count(), 2);

  auto domain_ptr = this->test_output_->domain_ptr();

  ASSERT_NE(domain_ptr, nullptr);
  EXPECT_NE(nullptr, dynamic_cast<domain::DefinitionMock<dim>*>(domain_ptr));
}

TYPED_TEST(ResultsOutputDealiiVtuTest, AddWriteDataTestMPI) {
  constexpr int dim = this->dim;
  std::ostringstream expected_stream, test_stream;

  this->test_data_out_.write_vtu(test_stream);
//  system::moments::MomentIndex group_0_index{0, 0, 0},
//      group_1_index{1, 0, 0};

//  EXPECT_CALL(*this->moments_obs_ptr_, GetMoment(group_0_index))
//      .WillOnce(ReturnRef(this->vector_1));
//  EXPECT_CALL(*this->moments_obs_ptr_, GetMoment(group_1_index))
//      .WillOnce(ReturnRef(this->vector_2));

  // Check that dof_handler is called to attach to data
  auto mock_domain_ptr = dynamic_cast<domain::DefinitionMock<dim>*>(
      this->test_output_->domain_ptr());
  ASSERT_NE(nullptr, mock_domain_ptr);

  EXPECT_CALL(*mock_domain_ptr, dof_handler())
      .WillOnce(ReturnRef(this->dof_handler_));

  this->test_output_->AddData(this->test_system_);
  this->test_output_->WriteData(expected_stream);

  EXPECT_EQ(test_stream.str(), expected_stream.str());
}

}