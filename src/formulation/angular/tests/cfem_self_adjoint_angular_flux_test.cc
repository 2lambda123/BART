#include "formulation/angular/cfem_self_adjoint_angular_flux.h"

#include <deal.II/base/tensor.h>

#include "data/cross_sections.h"
#include "domain/finite_element/tests/finite_element_mock.h"
#include "material/tests/mock_material.h"
#include "quadrature/tests/quadrature_set_mock.h"
#include "quadrature/tests/quadrature_point_mock.h"
#include "quadrature/utility/quadrature_utilities.h"
#include "system/system_types.h"
#include "test_helpers/gmock_wrapper.h"
#include "test_helpers/dealii_test_domain.h"
#include "test_helpers/test_assertions.h"

namespace  {

using namespace bart;

using ::testing::AssertionResult, ::testing::AssertionSuccess, ::testing::AssertionFailure;
using ::testing::DoDefault, ::testing::NiceMock, ::testing::Return;
using ::testing::_;

/* Tests for CFEM Self Adjoint Angular Flux formulation class. This class is
 * responsible for filling cell matrices for the SAAF angular formulation.
 *
 * Initial conditions: mock dependencies default call values set; dealii
 * test domain set up; cell pointer initialized to a locally owned cell with
 * material id set to material_id_;
 *
 * Dependencies are all shared pointers so these also function as observation
 * pointers.
 *
 */
template <typename DimensionWrapper>
class FormulationAngularCFEMSelfAdjointAngularFluxTest :
    public ::testing::Test ,
    bart::testing::DealiiTestDomain<DimensionWrapper::value> {
 public:
  static constexpr int dim = DimensionWrapper::value;

  typename dealii::DoFHandler<dim>::active_cell_iterator cell_ptr_;
  using FiniteElementType = typename domain::finite_element::FiniteElementMock<dim>;
  using QuadratureSetType = typename quadrature::QuadratureSetMock<dim>;
  using MaterialType = btest::MockMaterial;

  // Mock dependencies and supporting objects
  std::shared_ptr<FiniteElementType> mock_finite_element_ptr_;
  std::shared_ptr<QuadratureSetType> mock_quadrature_set_ptr_;
  std::shared_ptr<data::CrossSections> cross_section_ptr_;
  NiceMock<MaterialType> mock_material_;

  // Other test objects
  std::set<std::shared_ptr<quadrature::QuadraturePointI<dim>>,
           quadrature::utility::quadrature_point_compare<dim>> quadrature_set_;
  std::set<int> quadrature_point_indices_ = {};

  // Test parameters
  const int material_id_ = 1;
  // Cross-sections
  const std::unordered_map<int, std::vector<double>> sigma_t_{
      {material_id_, {1.0, 2.0}}};
  const std::unordered_map<int, std::vector<double>> inv_sigma_t_{
    {material_id_, {1.0, 0.5}}};
  const std::unordered_map<int, std::vector<double>> q_per_ster_{
      {material_id_, {1.0, 2.0}}};

  void SetUp() override;
};

// SETUP =======================================================================

/* SetUp initial test conditions.
 *
 * 1. Set default values for all mock objects.
 * 2. Set cell pointer to locally owned cell.
 */
template <typename DimensionWrapper>
void FormulationAngularCFEMSelfAdjointAngularFluxTest<DimensionWrapper>::SetUp() {
  this->SetUpDealii();

  // Make mocks and set up
  mock_finite_element_ptr_ = std::make_shared<NiceMock<FiniteElementType>>();
  mock_quadrature_set_ptr_ = std::make_shared<NiceMock<QuadratureSetType>>();

  // Set default return values
  ON_CALL(*mock_finite_element_ptr_, dofs_per_cell())
      .WillByDefault(Return(2));
  ON_CALL(*mock_finite_element_ptr_, n_cell_quad_pts())
      .WillByDefault(Return(2));
  ON_CALL(*mock_finite_element_ptr_, n_face_quad_pts())
      .WillByDefault(Return(2));
  // Set default return values for shape functions and Jacobian
  for (int quad_pt_idx = 0; quad_pt_idx < 2; ++quad_pt_idx) {
    ON_CALL(*mock_finite_element_ptr_, Jacobian(quad_pt_idx))
        .WillByDefault(Return(3*(quad_pt_idx + 1)));
    for (int dof_idx = 0; dof_idx < 2; ++dof_idx) {
      int entry = quad_pt_idx + 1 + 10*(dof_idx + 1);
      ON_CALL(*mock_finite_element_ptr_, ShapeValue(dof_idx, quad_pt_idx))
          .WillByDefault(Return(entry));
      dealii::Tensor<1, dim> gradient_entry;
      for (int i = 0; i < dim; ++i)
        gradient_entry[i] = (entry);
      ON_CALL(*mock_finite_element_ptr_, ShapeGradient(dof_idx, quad_pt_idx))
          .WillByDefault(Return(gradient_entry));
    }
  }

  // Set up mock quadrature points for quadrature set
  for (int n_angle = 0; n_angle < 2; ++n_angle) {
    auto new_quadrature_point =
        std::make_shared<NiceMock<quadrature::QuadraturePointMock<dim>>>();
    std::array<double, dim> position;
    position.fill(n_angle + 1);
    dealii::Tensor<1, dim> tensor_position;
    for (int i = 0; i < dim; ++i)
      tensor_position[i] = (n_angle + 1);
    ON_CALL(*new_quadrature_point, cartesian_position())
        .WillByDefault(Return(position));
    ON_CALL(*new_quadrature_point, cartesian_position_tensor())
        .WillByDefault(Return(tensor_position));
    ON_CALL(*mock_quadrature_set_ptr_,
        GetQuadraturePoint(quadrature::QuadraturePointIndex(n_angle)))
        .WillByDefault(Return(new_quadrature_point));

    auto return_pair = quadrature_set_.insert(new_quadrature_point);
    quadrature_point_indices_.insert(n_angle);

    ON_CALL(*mock_quadrature_set_ptr_,
            GetQuadraturePointIndex(*return_pair.first))
        .WillByDefault(Return(n_angle));
  }

  ON_CALL(*mock_quadrature_set_ptr_, quadrature_point_indices())
      .WillByDefault(Return(quadrature_point_indices_));
  ON_CALL(*mock_quadrature_set_ptr_, begin())
      .WillByDefault(Return(quadrature_set_.begin()));
  ON_CALL(*mock_quadrature_set_ptr_, end())
      .WillByDefault(Return(quadrature_set_.end()));

  // Set up cross-sections
  ON_CALL(mock_material_, GetSigT())
      .WillByDefault(Return(sigma_t_));
  ON_CALL(mock_material_, GetInvSigT())
      .WillByDefault(Return(inv_sigma_t_));
  ON_CALL(mock_material_, GetQPerSter())
      .WillByDefault(Return(q_per_ster_));

  // Instantiate cross-section object
  cross_section_ptr_ = std::make_shared<data::CrossSections>(mock_material_);

  // Find an active, locally owned cell and set the material ID
  for (auto cell = this->dof_handler_.begin_active();
       cell != this->dof_handler_.end(); ++cell) {
    if (cell->is_locally_owned()) {
      cell_ptr_ = cell;
      cell_ptr_->set_material_id(material_id_);
      break;
    }
  }
}

TYPED_TEST_CASE(FormulationAngularCFEMSelfAdjointAngularFluxTest,
                bart::testing::AllDimensions);

// HELPER FUNCTIONS ============================================================
AssertionResult CompareMatrices(const dealii::FullMatrix<double>& expected,
                                const dealii::FullMatrix<double>& result,
                                const double tol = 1e-6) {
  unsigned int rows = expected.m();
  unsigned int cols = expected.n();

  if (result.m() != rows)
    return AssertionFailure() << "Result has wrong number of rows: "
                              << result.m() << ", expected" << rows;
  if (result.n() != cols)
    return AssertionFailure() << "Result has wrong number of columns: "
                              << result.n() << ", expected" << cols;

  for (unsigned int i = 0; i < rows; ++i) {
    for (unsigned int j = 0; j < cols; ++j) {
      if (abs(result(i, j) - expected(i, j)) > tol) {
        return AssertionFailure() << "Entry (" << i << ", " << j <<
                                  ") has value: " << result(i, j) <<
                                  ", expected: " << expected(i, j);
      }
    }
  }
  return AssertionSuccess();
}

// TESTS =======================================================================

// CONSTRUCTOR AND CONST ACCESSORS

// Constructor should query appropriate values from finite element object
TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest, Constructor) {
  constexpr int dim = this->dim;

  EXPECT_CALL(*this->mock_finite_element_ptr_, dofs_per_cell())
      .WillOnce(::testing::DoDefault());
  EXPECT_CALL(*this->mock_finite_element_ptr_, n_cell_quad_pts())
      .WillOnce(::testing::DoDefault());
  EXPECT_CALL(*this->mock_finite_element_ptr_, n_face_quad_pts())
      .WillOnce(::testing::DoDefault());

  formulation::angular::CFEMSelfAdjointAngularFlux<dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);
}

// Getters should return observation pointers to dependencies
TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest,
    DependencyPointers) {
  constexpr int dim = this->dim;

  formulation::angular::CFEMSelfAdjointAngularFlux<dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);

  EXPECT_EQ(test_saaf.finite_element_ptr(), this->mock_finite_element_ptr_.get());
  EXPECT_EQ(test_saaf.cross_sections_ptr(), this->cross_section_ptr_.get());
  EXPECT_EQ(test_saaf.quadrature_set_ptr(), this->mock_quadrature_set_ptr_.get());
}

// FUNCTION TESTS: Initialize

// Initialize should calculate the correct matrices for shape squared
TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest,
    InitializeValuesShapeSquared) {
  constexpr int dim = this->dim;

  formulation::angular::CFEMSelfAdjointAngularFlux<dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);

  // Quadrature point shape-squared matrix expected values based on
  // hand-calculating the values from shape(i,q) = q + 1 + 10*(i + 1)
  formulation::FullMatrix expected_shape_squared_q_0{
    2,2, std::array<double, 4>{121, 231, 231, 441}.begin()};
  formulation::FullMatrix expected_shape_squared_q_1{
      2,2, std::array<double, 4>{144, 264, 264, 484}.begin()};

  EXPECT_CALL(*this->mock_finite_element_ptr_, SetCell(this->cell_ptr_))
      .Times(1);
  EXPECT_CALL(*this->mock_finite_element_ptr_, ShapeValue(_,_))
      .Times(16)
      .WillRepeatedly(DoDefault());

  EXPECT_NO_THROW(test_saaf.Initialize(this->cell_ptr_));
  auto shape_squared = test_saaf.shape_squared();

  ASSERT_EQ(shape_squared.size(), 2);
  EXPECT_TRUE(CompareMatrices(expected_shape_squared_q_0, shape_squared.at(0)));
  EXPECT_TRUE(CompareMatrices(expected_shape_squared_q_1, shape_squared.at(1)));
}

// Initialize should calculate the correct matrices for Omega dot gradient vectors
TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest,
    InitializeOmegaDotGradient) {
  constexpr int dim = this->dim;

  formulation::angular::CFEMSelfAdjointAngularFlux<dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);

  /* Procedure should be: get all the quadrature point indices, retrieve each
   * quadrature point using indices, get the position tensor and multiply by
   * the gradient shape. This should be repeated for each degree of freedom
   * (i.e. twice). */
  EXPECT_CALL(*this->mock_finite_element_ptr_, SetCell(this->cell_ptr_))
      .Times(1);
  EXPECT_CALL(*this->mock_quadrature_set_ptr_, quadrature_point_indices())
      .Times(::testing::AtLeast(1))
      .WillRepeatedly(DoDefault());

  for (int index : this->quadrature_point_indices_) {
    EXPECT_CALL(*this->mock_quadrature_set_ptr_,
                GetQuadraturePoint(quadrature::QuadraturePointIndex(index)))
        .Times(2)
        .WillRepeatedly(DoDefault());
  }

  for (auto quadrature_point_ptr : this->quadrature_set_) {
    auto mock_quadrature_point_ptr =
        dynamic_cast<quadrature::QuadraturePointMock<dim>*>(quadrature_point_ptr.get());
    EXPECT_CALL(*mock_quadrature_point_ptr, cartesian_position_tensor())
        .Times(4)
        .WillRepeatedly(DoDefault());
  }

  for (int quad_pt_idx = 0; quad_pt_idx < 2; ++quad_pt_idx) {
    for (int dof_idx = 0; dof_idx < 2; ++dof_idx) {
      EXPECT_CALL(*this->mock_finite_element_ptr_,
          ShapeGradient(quad_pt_idx, dof_idx))
          .Times(2)
          .WillRepeatedly(DoDefault());
    }
  }

  /* Expected values: For each quadrature point (with a given omega) there should
   * be two entries, a value for each degree of freedom. */

  std::map<int, std::map<int, std::vector<double>>> omega_dot_gradient{
      {0, {{0, {11*dim, 21*dim}}, {1, {22*dim, 42*dim}}}},
      {1, {{0, {12*dim, 22*dim}}, {1, {24*dim, 44*dim}}}}
  };

  EXPECT_NO_THROW(test_saaf.Initialize(this->cell_ptr_));

  for (int cell_quad_point = 0; cell_quad_point < 2; ++cell_quad_point) {
    for (int angle_index : this->quadrature_point_indices_) {
      std::vector<double> result;
      EXPECT_NO_THROW(result =
          test_saaf.OmegaDotGradient(
              cell_quad_point,
              quadrature::QuadraturePointIndex(angle_index)));
      EXPECT_THAT(result,
                  ::testing::ContainerEq(
                      omega_dot_gradient.at(cell_quad_point).at(angle_index)));
    }
  }
}

/* Initialize should generate the correct squares of the omega dot gradient
 * vectors. Returned as a matrix with entries corresponding to cell quadrature
 * points (i,j). No extra expectations are required, these are covered by
 * the above test. */
TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest,
    InitializeOmegaDotGradientSquared) {
  constexpr int dim = this->dim;

  double dim_sq = dim*dim;
  formulation::FullMatrix odgs_00{
      2,2, std::array<double, 4>{121, 231, 231, 441}.begin()};
  odgs_00 *= dim_sq;
  formulation::FullMatrix odgs_01{
      2,2, std::array<double, 4>{484, 924, 924, 1764}.begin()};
  odgs_01 *= dim_sq;
  formulation::FullMatrix odgs_10{
      2,2, std::array<double, 4>{144, 264, 264, 484}.begin()};
  odgs_10 *= dim_sq;
  formulation::FullMatrix odgs_11{
      2,2, std::array<double, 4>{576, 1056, 1056, 1936}.begin()};
  odgs_11 *= dim_sq;
  std::map<int, std::map<int, formulation::FullMatrix>>
      omega_dot_gradient_squared = {
      {0, {{0, odgs_00}, {1, odgs_01}}},
      {1, {{0, odgs_10}, {1, odgs_11}}}
      };

  formulation::angular::CFEMSelfAdjointAngularFlux<dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);
  test_saaf.Initialize(this->cell_ptr_);

  for (int cell_quad_point = 0; cell_quad_point < 2; ++cell_quad_point) {
    for (int angle_index : this->quadrature_point_indices_) {
      formulation::FullMatrix result;
      ASSERT_NO_THROW(result =
                          test_saaf.OmegaDotGradientSquared(
                              cell_quad_point,
                              quadrature::QuadraturePointIndex(angle_index)));
      EXPECT_TRUE(CompareMatrices(
          omega_dot_gradient_squared.at(cell_quad_point).at(angle_index),
          result));
    }
  }
}

// Initialize should throw an error if cell_ptr is invalid
TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest,
    InitializeBadCellPtr) {
  constexpr int dim = this->dim;

  formulation::angular::CFEMSelfAdjointAngularFlux<dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);

  formulation::CellPtr<dim> invalid_cell_ptr;
  EXPECT_ANY_THROW(test_saaf.Initialize(invalid_cell_ptr));
}
// =============================================================================
// FUNCTION TESTS:
// =============================================================================

// FillStreamingTerm ===========================================================
TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest,
    FillCellStreamingTermTestBadCellPtr) {
  constexpr int dim = this->dim;

  formulation::angular::CFEMSelfAdjointAngularFlux<dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);

  formulation::FullMatrix cell_matrix(2,2);
  auto angle_ptr = *this->quadrature_set_.begin();
  formulation::CellPtr<dim> invalid_cell_ptr;


  auto token = test_saaf.Initialize(this->cell_ptr_);
  EXPECT_ANY_THROW({
    test_saaf.FillCellStreamingTerm(cell_matrix, token, invalid_cell_ptr,
                                    angle_ptr, system::EnergyGroup(0));
  });
}

TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest,
           FillCellStreamingTermTestBadMatrixSize) {
  constexpr int dim = this->dim;

  formulation::angular::CFEMSelfAdjointAngularFlux<dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);

  formulation::FullMatrix bad_cell_matrix(3,2);
  auto angle_ptr = *this->quadrature_set_.begin();


  auto token = test_saaf.Initialize(this->cell_ptr_);
  EXPECT_ANY_THROW({
    test_saaf.FillCellStreamingTerm(bad_cell_matrix, token, this->cell_ptr_,
                                    angle_ptr, system::EnergyGroup(0));
                   });

  formulation::FullMatrix second_bad_cell_matrix(2,3);
  EXPECT_ANY_THROW({
    test_saaf.FillCellStreamingTerm(second_bad_cell_matrix, token,
                                    this->cell_ptr_, angle_ptr,
                                    system::EnergyGroup(0));
                   });
}


TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest,
    FillCellStreamingTermTest) {
  constexpr int dim = this->dim;

  formulation::angular::CFEMSelfAdjointAngularFlux<dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);

  formulation::FullMatrix cell_matrix(2,2);
  auto token = test_saaf.Initialize(this->cell_ptr_);

  std::map<std::pair<int, int>, formulation::FullMatrix> expected_results;
  formulation::FullMatrix expected_result_g0_a0(
      2, 2, std::array<double, 4>{1227, 2277, 2277, 4227}.begin());
  formulation::FullMatrix expected_result_g1_a0(
      2, 2, std::array<double, 4>{613.5, 1138.5, 1138.5, 2113.5}.begin());
  formulation::FullMatrix expected_result_g0_a1(
      2, 2, std::array<double, 4>{4908, 9108, 9108, 16908}.begin());
  formulation::FullMatrix expected_result_g1_a1(
      2, 2, std::array<double, 4>{2454, 4554, 4554, 8454}.begin());
  expected_results.insert_or_assign({0,0}, expected_result_g0_a0);
  expected_results.insert_or_assign({1,0}, expected_result_g1_a0);
  expected_results.insert_or_assign({0,1}, expected_result_g0_a1);
  expected_results.insert_or_assign({1,1}, expected_result_g1_a1);

  for (auto& result : expected_results)
    result.second *= dim*dim;

  for (int group = 0; group < 2; ++group) {
    for (int angle = 0; angle < 2; ++angle) {
      auto angle_it = this->quadrature_set_.begin();
      if (angle == 1)
        ++angle_it;
      auto angle_ptr = *angle_it;
      EXPECT_CALL(*this->mock_finite_element_ptr_, SetCell(this->cell_ptr_));
      EXPECT_CALL(*this->mock_finite_element_ptr_, Jacobian(_))
          .Times(2)
          .WillRepeatedly(DoDefault());
      EXPECT_CALL(*this->mock_quadrature_set_ptr_, GetQuadraturePointIndex(_))
          .WillOnce(DoDefault());

      cell_matrix = 0;

      EXPECT_NO_THROW({
        test_saaf.FillCellStreamingTerm(cell_matrix, token, this->cell_ptr_,
                                        angle_ptr, system::EnergyGroup(group));
                      });
      std::pair<int, int> result_index{group, angle};
      EXPECT_TRUE(CompareMatrices(expected_results.at(result_index),
                                  cell_matrix))
                << "Failed: group: " << group << " angle: " << angle;
    }
  }
}

// FillCollisionTerm ===========================================================

TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest,
           FillCellCollisionTermTestBadCellPtr) {
  constexpr int dim = this->dim;
  formulation::angular::CFEMSelfAdjointAngularFlux<dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);

  formulation::FullMatrix cell_matrix(2,2);
  formulation::CellPtr<dim> invalid_cell_ptr;
  auto token = test_saaf.Initialize(this->cell_ptr_);
  EXPECT_ANY_THROW({test_saaf.FillCellCollisionTerm(cell_matrix, token,
                                                    invalid_cell_ptr,
                                                    system::EnergyGroup(0));});
}

TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest,
           FillCellCollisionTermTestBadMatrixSize) {
  constexpr int dim = this->dim;
  formulation::angular::CFEMSelfAdjointAngularFlux<dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);

  formulation::FullMatrix bad_cell_matrix(3,2), second_bad_cell_matrix(2,3);;
  auto token = test_saaf.Initialize(this->cell_ptr_);
  EXPECT_ANY_THROW({test_saaf.FillCellCollisionTerm(bad_cell_matrix, token,
                                                    this->cell_ptr_,
                                                    system::EnergyGroup(0));});
  EXPECT_ANY_THROW({test_saaf.FillCellCollisionTerm(second_bad_cell_matrix,
                                                    token, this->cell_ptr_,
                                                    system::EnergyGroup(0));});
}

TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest,
    FillCellCollisionTermTest) {
  formulation::angular::CFEMSelfAdjointAngularFlux<this->dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);

  auto token = test_saaf.Initialize(this->cell_ptr_);

  for (int group = 0; group < 2; ++group) {
    EXPECT_CALL(*this->mock_finite_element_ptr_, SetCell(this->cell_ptr_));
    EXPECT_CALL(*this->mock_finite_element_ptr_, Jacobian(_))
        .Times(2)
        .WillRepeatedly(DoDefault());
    EXPECT_CALL(*this->mock_finite_element_ptr_, ShapeValue(_,_))
        .Times(16)
        .WillRepeatedly(DoDefault());

    formulation::FullMatrix cell_matrix(2,2), expected_result(
        2,2, std::array<double, 4>{1227, 2277, 2277, 4227}.begin());
    expected_result *= this->sigma_t_.at(this->material_id_).at(group);

    EXPECT_NO_THROW({test_saaf.FillCellCollisionTerm(cell_matrix, token,
                                                     this->cell_ptr_,
                                                     system::EnergyGroup(group));
    });
    EXPECT_TRUE(CompareMatrices(expected_result, cell_matrix));
  }
}

// FillCellFixedSourceTerm =====================================================

TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest,
           FillCellFixedSourceTermBadCell) {
  constexpr int dim = this->dim;
  formulation::angular::CFEMSelfAdjointAngularFlux<dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);

  formulation::Vector cell_vector(2);
  formulation::CellPtr<dim> invalid_cell_ptr;
  auto angle_ptr = *this->quadrature_set_.begin();
  auto token = test_saaf.Initialize(this->cell_ptr_);

  EXPECT_ANY_THROW({
    test_saaf.FillCellFixedSourceTerm(cell_vector, token, invalid_cell_ptr,
                                    angle_ptr, system::EnergyGroup(0));
  });
}

TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest,
           FillCellFixedSourceTermBadVectorLength) {
  constexpr int dim = this->dim;
  formulation::angular::CFEMSelfAdjointAngularFlux<dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);

  formulation::Vector bad_cell_vector(3);
  auto angle_ptr = *this->quadrature_set_.begin();
  auto token = test_saaf.Initialize(this->cell_ptr_);

  EXPECT_ANY_THROW({
    test_saaf.FillCellFixedSourceTerm(bad_cell_vector, token, this->cell_ptr_,
                                      angle_ptr, system::EnergyGroup(0));
                   });
}

TYPED_TEST(FormulationAngularCFEMSelfAdjointAngularFluxTest,
           FillCellFixedSourceTerm) {
  constexpr int dim = this->dim;

  formulation::angular::CFEMSelfAdjointAngularFlux<dim> test_saaf(
      this->mock_finite_element_ptr_,
      this->cross_section_ptr_,
      this->mock_quadrature_set_ptr_);

  formulation::Vector cell_vector(2);
  auto token = test_saaf.Initialize(this->cell_ptr_);

  double d = this->dim;

  std::map<std::pair<int, int>, formulation::Vector> expected_results;

  formulation::Vector expected_result_g0_a0(2), expected_result_g0_a1(2),
      expected_result_g1_a0(2), expected_result_g1_a1(2);
  expected_result_g0_a0[0] = 105.0 * (d + 1.0);
  expected_result_g0_a0[1] = 195.0*(d + 1.0);
  expected_result_g0_a1[0] = 105.0 + 210.0*d;
  expected_result_g0_a1[1] = 195.0 + 390.0*d;
  expected_result_g1_a0[0] = 210.0 + 105.0*d;
  expected_result_g1_a0[1] = 390.0 + 195.0*d;
  expected_result_g1_a1[0] = 210.0*(d + 1.0);
  expected_result_g1_a1[1] = 390.0*(d + 1.0);

  expected_results.insert_or_assign({0,0}, expected_result_g0_a0);
  expected_results.insert_or_assign({1,0}, expected_result_g1_a0);
  expected_results.insert_or_assign({0,1}, expected_result_g0_a1);
  expected_results.insert_or_assign({1,1}, expected_result_g1_a1);

  for (int group = 0; group < 2; ++group) {
    for (int angle = 0; angle < 2; ++angle) {
      auto angle_it = this->quadrature_set_.begin();
      if (angle == 1)
        ++angle_it;
      auto angle_ptr = *angle_it;
      EXPECT_CALL(*this->mock_finite_element_ptr_, SetCell(this->cell_ptr_));
      EXPECT_CALL(*this->mock_finite_element_ptr_, Jacobian(_))
          .Times(2)
          .WillRepeatedly(DoDefault());
      EXPECT_CALL(*this->mock_quadrature_set_ptr_, GetQuadraturePointIndex(_))
          .WillOnce(DoDefault());

      cell_vector = 0;

      EXPECT_NO_THROW({
        test_saaf.FillCellFixedSourceTerm(cell_vector, token, this->cell_ptr_,
                                          angle_ptr,
                                          system::EnergyGroup(group));
                      });
      std::pair<int, int> result_index{group, angle};
      EXPECT_EQ(expected_results.at(result_index), cell_vector)
                << "Failed: group: " << group << " angle: " << angle;
    }
  }
}

} // namespace