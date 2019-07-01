#include "calculator/cell/fission_source_norm.h"

#include <memory>

#include "data/cross_sections.h"
#include "domain/domain_types.h"
#include "domain/tests/finite_element_mock.h"
#include "material/tests/mock_material.h"
#include "test_helpers/gmock_wrapper.h"


namespace  {

using namespace bart;

using ::testing::Return, ::testing::DoDefault, ::testing::NiceMock;

template <typename DimensionWrapper>
class CalcCellFissionSourceNormTest :public ::testing::Test {
 protected:
  static constexpr int dim = DimensionWrapper::value;
  using FiniteElementType = typename domain::FiniteElementMock<dim>;
  using FissionSourceNormType = typename calculator::cell::FissionSourceNorm<dim>;

  domain::CellPtr<dim> cell_ptr_;

  // Supporting objects and mocks
  std::shared_ptr<NiceMock<FiniteElementType>> finite_element_ptr_;
  std::shared_ptr<data::CrossSections> cross_sections_ptr_;


  NiceMock<btest::MockMaterial> mock_material_;

  void SetUp() override;
};

template <typename DimensionWrapper>
void CalcCellFissionSourceNormTest<DimensionWrapper>::SetUp() {
  finite_element_ptr_ = std::make_shared<NiceMock<FiniteElementType>>();
  cross_sections_ptr_ = std::make_shared<data::CrossSections>(mock_material_);

  ON_CALL(*finite_element_ptr_, n_cell_quad_pts())
      .WillByDefault(Return(4));
}

TYPED_TEST_CASE(CalcCellFissionSourceNormTest, bart::testing::AllDimensions);

TYPED_TEST(CalcCellFissionSourceNormTest, Constructor) {
  static constexpr int dim = this->dim;

  EXPECT_CALL(*this->finite_element_ptr_, n_cell_quad_pts())
      .WillOnce(DoDefault());

  calculator::cell::FissionSourceNorm<dim> test_calculator(
      this->finite_element_ptr_,
      this->cross_sections_ptr_);

  EXPECT_EQ(this->finite_element_ptr_.use_count(), 2);
  EXPECT_EQ(this->cross_sections_ptr_.use_count(), 2);
}

TYPED_TEST(CalcCellFissionSourceNormTest, GetCellNorm) {
  static constexpr int dim = this->dim;
  auto& finite_element_mock = *(this->finite_element_ptr_);

  EXPECT_CALL(finite_element_mock, SetCell(this->cell_ptr_));

  calculator::cell::FissionSourceNorm<dim> test_calculator(
      this->finite_element_ptr_,
      this->cross_sections_ptr_);

  test_calculator.GetCellNorm(this->cell_ptr_);
}

} // namespace