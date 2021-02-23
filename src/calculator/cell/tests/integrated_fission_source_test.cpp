#include "calculator/cell/integrated_fission_source.hpp"

#include <memory>

#include "data/cross_sections/material_cross_sections.hpp"
#include "domain/finite_element/tests/finite_element_mock.hpp"
#include "data/material/tests/material_mock.hpp"
#include "system/moments/spherical_harmonic_types.h"
#include "system/moments/tests/spherical_harmonic_mock.h"
#include "test_helpers/gmock_wrapper.h"
#include "test_helpers/dealii_test_domain.h"

namespace  {

using namespace bart;

using ::testing::Return, ::testing::ReturnRef, ::testing::DoDefault, ::testing::NiceMock;

template <typename DimensionWrapper>
class CalcCellIntegratedFissionSourceTest : public ::testing::Test,
                                            public bart::testing::DealiiTestDomain<DimensionWrapper::value> {
 protected:
  static constexpr int dim = DimensionWrapper::value;
  using FiniteElementType = typename domain::finite_element::FiniteElementMock<dim>;
  using SphericalHarmonicType = system::moments::SphericalHarmonicMock;

  // Supporting objects and mocks
  std::shared_ptr<NiceMock<FiniteElementType>> finite_element_ptr_;
  std::shared_ptr<data::cross_sections::MaterialCrossSections> cross_sections_ptr_;
  std::shared_ptr<SphericalHarmonicType > spherical_harmonic_ptr_;

  NiceMock<data::material::MaterialMock> mock_material_;

  // test parameters
  static constexpr int quadrature_points_{ 4 };

  void SetUp() override;
};

template <typename DimensionWrapper>
void CalcCellIntegratedFissionSourceTest<DimensionWrapper>::SetUp() {
  std::unordered_map<int, bool> fissile_id_map{{0, true}, {1, false}};
  std::unordered_map<int, std::vector<double>> nu_sigma_f = {{0, {1.0, 2.0}}, {1, {3.0, 6.0}}};

  ON_CALL(mock_material_, GetFissileIDMap()).WillByDefault(Return(fissile_id_map));
  ON_CALL(mock_material_, GetNuSigF()).WillByDefault(Return(nu_sigma_f));

  finite_element_ptr_ = std::make_shared<NiceMock<FiniteElementType>>();
  cross_sections_ptr_ = std::make_shared<data::cross_sections::MaterialCrossSections>(mock_material_);
  spherical_harmonic_ptr_ = std::make_shared<SphericalHarmonicType>();

  ON_CALL(*finite_element_ptr_, n_cell_quad_pts()).WillByDefault(Return(this->quadrature_points_));

  this->SetUpDealii();
}

TYPED_TEST_CASE(CalcCellIntegratedFissionSourceTest, bart::testing::AllDimensions);

/* Constructor should call n_cell_quad_pts to store this value, and set dependencies correctly */
TYPED_TEST(CalcCellIntegratedFissionSourceTest, Constructor) {
  static constexpr int dim = this->dim;

  EXPECT_CALL(*this->finite_element_ptr_, n_cell_quad_pts()).WillOnce(DoDefault());

  calculator::cell::IntegratedFissionSource<dim> test_calculator(this->finite_element_ptr_, this->cross_sections_ptr_);
  EXPECT_EQ(test_calculator.finite_element_ptr(), this->finite_element_ptr_.get());
  EXPECT_EQ(test_calculator.cross_sections_ptr(), this->cross_sections_ptr_.get());
}

/* Constructor should throw an error if dependencies are null */
TYPED_TEST(CalcCellIntegratedFissionSourceTest, ConstructorNullDependencies) {
  static constexpr int dim = this->dim;
  using IntegratedFissionSource = calculator::cell::IntegratedFissionSource<dim>;

  EXPECT_CALL(*this->finite_element_ptr_, n_cell_quad_pts()).WillOnce(DoDefault());

  EXPECT_ANY_THROW({ IntegratedFissionSource test_calculator(nullptr, this->cross_sections_ptr_); });
  EXPECT_ANY_THROW({ IntegratedFissionSource test_calculator(this->finite_element_ptr_, nullptr); });
}

/* Calculating the fission source for a non-fissile material should return 0 */
TYPED_TEST(CalcCellIntegratedFissionSourceTest, CellValueNonFissileMPI) {
  static constexpr int dim = this->dim;
  auto& cell{ this->cells_.at(0) };
  cell->set_material_id(1); // Set to non-fissile material id
  calculator::cell::IntegratedFissionSource<dim> test_calculator(this->finite_element_ptr_,
                                                                 this->cross_sections_ptr_);
  EXPECT_DOUBLE_EQ(test_calculator.CellValue(cell, this->spherical_harmonic_ptr_.get()), 0.0);
}

/* Calculating the fission source for a fissile material should return the correct value. */
TYPED_TEST(CalcCellIntegratedFissionSourceTest, CellValueMPI) {
  static constexpr int dim = this->dim;
  auto& finite_element_mock = *(this->finite_element_ptr_);
  auto& spherical_harmonic_mock = *(this->spherical_harmonic_ptr_);

  auto& cell{ this->cells_.at(0) };
  cell->set_material_id(0);  // Set to the fissile material id

  for (int q = 0; q < this->quadrature_points_; ++q) {
    EXPECT_CALL(finite_element_mock, Jacobian(q)).Times(2).WillRepeatedly(Return(10*(q+1)));
  }
  // Number of groups should be determined by Spherical Harmonics
  EXPECT_CALL(spherical_harmonic_mock, total_groups()).WillOnce(Return(2));
  // Zeroth moments should be queried for each group
  std::vector<double> group_0_flux{1, 2, 3, 4}, group_1_flux{4, 3, 2, 1};

  system::moments::MomentVector group_0_flux_vector(group_0_flux.begin(), group_0_flux.end());
  system::moments::MomentVector group_1_flux_vector(group_1_flux.begin(), group_1_flux.end());

  system::moments::MomentIndex group_0_index{0,0,0};
  system::moments::MomentIndex group_1_index{1,0,0};

  EXPECT_CALL(spherical_harmonic_mock, GetMoment(group_0_index)).WillOnce(ReturnRef(group_0_flux_vector));
  EXPECT_CALL(spherical_harmonic_mock, GetMoment(group_1_index)).WillOnce(ReturnRef(group_1_flux_vector));
  EXPECT_CALL(finite_element_mock, ValueAtQuadrature(group_0_flux_vector)).WillOnce(Return(group_0_flux));
  EXPECT_CALL(finite_element_mock, ValueAtQuadrature(group_1_flux_vector)).WillOnce(Return(group_1_flux));

  calculator::cell::IntegratedFissionSource<dim> test_calculator(this->finite_element_ptr_,
                                                                 this->cross_sections_ptr_);

  double fission_norm = test_calculator.CellValue(cell, this->spherical_harmonic_ptr_.get());
  EXPECT_DOUBLE_EQ(fission_norm, 700.0);
}




} // namespace