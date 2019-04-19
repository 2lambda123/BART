#include "formulation/scalar/cfem_diffusion.h"

#include <array>
#include <memory>
#include <cstdlib>

#include <deal.II/dofs/dof_handler.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/tria.h>

#include "data/cross_sections.h"
#include "domain/tests/finite_element_mock.h"
#include "material/tests/mock_material.h"
#include "test_helpers/gmock_wrapper.h"
#include "test_helpers/test_helper_functions.h"


namespace {

using ::testing::AssertionResult;
using ::testing::AssertionFailure;
using ::testing::AssertionSuccess;
using ::testing::Le;
using ::testing::Ge;
using ::testing::DoDefault;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;
using namespace bart;

class FormulationCFEMDiffusionTest : public ::testing::Test {
 protected:
  FormulationCFEMDiffusionTest();
  using Matrix = dealii::FullMatrix<double>;
  std::shared_ptr<domain::FiniteElementMock<2>> fe_mock_ptr;
  std::shared_ptr<data::CrossSections> cross_sections_ptr;

  dealii::DoFHandler<2>::active_cell_iterator cell_ptr_;
  dealii::Triangulation<2> triangulation_;
  dealii::DoFHandler<2> dof_handler_;
  dealii::FE_Q<2> fe_;

  void SetUp() override;
  void SetUpDealii();
};

FormulationCFEMDiffusionTest::FormulationCFEMDiffusionTest()
    : dof_handler_(triangulation_),
      fe_(1) {}

void FormulationCFEMDiffusionTest::SetUp() {
  SetUpDealii();
  // Make mock objects. Cross-sections is a struct that cannot be mocked, but
  // we can mock the material object it is based on.
  NiceMock<btest::MockMaterial> mock_material;
  fe_mock_ptr = std::make_shared<NiceMock<domain::FiniteElementMock<2>>>();

  ON_CALL(*fe_mock_ptr, dofs_per_cell())
      .WillByDefault(Return(2));
  ON_CALL(*fe_mock_ptr, n_cell_quad_pts())
      .WillByDefault(Return(2));
  ON_CALL(*fe_mock_ptr, n_face_quad_pts())
      .WillByDefault(Return(2));

  for (int q = 0; q < 2; ++q) {
    ON_CALL(*fe_mock_ptr, Jacobian(q))
        .WillByDefault(Return((1 + q)*3));
    for (int i = 0; i < 2; ++i) {
      ON_CALL(*fe_mock_ptr, ShapeValue(i, q))
          .WillByDefault(Return(i + q));

      dealii::Tensor<1, 2> gradient_tensor;
      gradient_tensor[0] = i;
      gradient_tensor[1] = q;

      ON_CALL(*fe_mock_ptr, ShapeGradient(i, q))
          .WillByDefault(Return(gradient_tensor));
    }
  }

  // Cross-section data for fake two-group, with one material (id = 0)
  std::array<double, 4> sigma_s_values{0.25, 0.5, 0.75, 1.0};
  dealii::FullMatrix<double> sigma_s_matrix{2,2, sigma_s_values.begin()};
  std::unordered_map<int, dealii::FullMatrix<double>> sigma_s{{0, sigma_s_matrix}};

  std::unordered_map<int, std::vector<double>> sigma_t{{0, {1.0, 2.0}}}; // i + 1

  std::unordered_map<int, std::vector<double>> q{{0, {1.0, 2.0}}};

  std::unordered_map<int, bool> fissile_id{{0, true}};

  ON_CALL(mock_material, GetSigT())
      .WillByDefault(Return(sigma_t));
  ON_CALL(mock_material, GetSigS())
      .WillByDefault(Return(sigma_s));
  ON_CALL(mock_material, GetDiffusionCoef())
      .WillByDefault(Return(sigma_t));
  ON_CALL(mock_material, GetQ())
      .WillByDefault(Return(q));
  ON_CALL(mock_material, GetFissileIDMap())
      .WillByDefault(Return(fissile_id));
  ON_CALL(mock_material, GetChiNuSigF())
      .WillByDefault(Return(sigma_s));

  cross_sections_ptr = std::make_shared<data::CrossSections>(mock_material);
}

// Set up a simple deal.ii problem so that the cell points to something, this
// is for material ID
void FormulationCFEMDiffusionTest::SetUpDealii() {
  dealii::GridGenerator::hyper_cube(triangulation_, 0, 1);
  dof_handler_.distribute_dofs(fe_);
  for (auto cell = dof_handler_.begin_active();
       cell != dof_handler_.end();
       ++cell) {
    if (cell->is_locally_owned()) {
      cell_ptr_ = cell;
      cell_ptr_->set_material_id(0);
    }
  }
}


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

AssertionResult CompareVector(const dealii::Vector<double>& expected,
                              const dealii::Vector<double>& result,
                              const double tol = 1e-6) {
  unsigned int size = expected.size();

  if (result.size() != size)
    return AssertionFailure() << "Result has wrong number of entries: "
                              << result.size() << ", expected" << size;

  for (unsigned int i = 0; i < size; ++i) {
    if (abs(result[i] - expected[i]) > tol) {
      return AssertionFailure() << "Entry (" << i <<
                                ") has value: " << result[i] <<
                                ", expected: " << expected[i];
    }
  }
  return AssertionSuccess();
}


TEST_F(FormulationCFEMDiffusionTest, ConstructorTest) {
  EXPECT_CALL(*fe_mock_ptr, dofs_per_cell())
      .WillOnce(DoDefault());
  EXPECT_CALL(*fe_mock_ptr, n_cell_quad_pts())
      .WillOnce(DoDefault());
  EXPECT_CALL(*fe_mock_ptr, n_face_quad_pts())
      .WillOnce(DoDefault());

  formulation::scalar::CFEM_Diffusion<2> test_diffusion(fe_mock_ptr,
                                                        cross_sections_ptr);

  EXPECT_EQ(fe_mock_ptr.use_count(), 2);
  EXPECT_EQ(cross_sections_ptr.use_count(), 2);
}

TEST_F(FormulationCFEMDiffusionTest, PrecalculateTest) {
  // Shape call, by default, returns (quadrature point index + degree of freedom)
  // Gradient call will return a tensor of [i, q]
  // Expected results, we need to make arrays to put them into dealii matrices
  std::array<double, 4> shape_matrix_q_0_values = {0, 0,
                                                   0, 1};
  std::array<double, 4> shape_matrix_q_1_values = {1, 2,
                                                   2, 4};
  std::array<double, 4> gradient_matrix_q_0_values = {0, 0,
                                                      0, 1};
  std::array<double, 4> gradient_matrix_q_1_values = {1, 1,
                                                      1, 2};

  dealii::FullMatrix<double> shape_matrix_q_0{2,2,
                                              shape_matrix_q_0_values.begin()};
  dealii::FullMatrix<double> shape_matrix_q_1{2,2,
                                              shape_matrix_q_1_values.begin()};
  dealii::FullMatrix<double> gradient_matrix_q_0{2,2,
                                                 gradient_matrix_q_0_values.begin()};
  dealii::FullMatrix<double> gradient_matrix_q_1{2,2,
                                                 gradient_matrix_q_1_values.begin()};

  formulation::scalar::CFEM_Diffusion<2> test_diffusion(fe_mock_ptr,
                                                        cross_sections_ptr);

  // Set call expectations
  EXPECT_CALL(*fe_mock_ptr, SetCell(_))
      .Times(1);
  EXPECT_CALL(*fe_mock_ptr, ShapeValue(_,_))
      .Times(16)
      .WillRepeatedly(DoDefault());
  EXPECT_CALL(*fe_mock_ptr, ShapeGradient(_,_))
      .Times(16)
      .WillRepeatedly(DoDefault());

  test_diffusion.Precalculate(cell_ptr_);
  auto shape_squared = test_diffusion.GetShapeSquared();
  auto gradient_squared = test_diffusion.GetGradientSquared();

  EXPECT_TRUE(CompareMatrices(shape_matrix_q_0, shape_squared.at(0)));
  EXPECT_TRUE(CompareMatrices(shape_matrix_q_1, shape_squared.at(1)));
  EXPECT_TRUE(CompareMatrices(gradient_matrix_q_0, gradient_squared.at(0)));
  EXPECT_TRUE(CompareMatrices(gradient_matrix_q_1, gradient_squared.at(1)));
}

TEST_F(FormulationCFEMDiffusionTest, FillCellStreamingTermTest) {
  dealii::FullMatrix<double> test_matrix(2,2);

  std::array<double, 4> expected_values{6, 6,
                                        6, 15};
  dealii::FullMatrix<double> expected_matrix(2, 2, expected_values.begin());

  formulation::scalar::CFEM_Diffusion<2> test_diffusion(fe_mock_ptr,
                                                        cross_sections_ptr);

  auto init_token = test_diffusion.Precalculate(cell_ptr_);

  EXPECT_CALL(*fe_mock_ptr, Jacobian(_))
      .Times(2)
      .WillRepeatedly(DoDefault());
  EXPECT_CALL(*fe_mock_ptr, SetCell(cell_ptr_))
      .Times(1);

  test_diffusion.FillCellStreamingTerm(test_matrix, init_token, cell_ptr_, 0);

  EXPECT_TRUE(CompareMatrices(expected_matrix, test_matrix));
}

TEST_F(FormulationCFEMDiffusionTest, FillCellCollisionTermTest) {
  dealii::FullMatrix<double> test_matrix(2,2);

  std::array<double, 4> expected_values{4.5, 9.0,
                                        9.0, 20.25};
  dealii::FullMatrix<double> expected_matrix(2, 2, expected_values.begin());

  formulation::scalar::CFEM_Diffusion<2> test_diffusion(fe_mock_ptr,
                                                        cross_sections_ptr);

  auto init_token = test_diffusion.Precalculate(cell_ptr_);

  EXPECT_CALL(*fe_mock_ptr, Jacobian(_))
      .Times(2)
      .WillRepeatedly(DoDefault());
  EXPECT_CALL(*fe_mock_ptr, SetCell(cell_ptr_))
      .Times(1);

  test_diffusion.FillCellCollisionTerm(test_matrix, init_token, cell_ptr_, 0);

  EXPECT_TRUE(CompareMatrices(expected_matrix, test_matrix));
}

TEST_F(FormulationCFEMDiffusionTest, FillBoundaryTermTestReflective) {
  dealii::FullMatrix<double> test_matrix(2,2);
  dealii::FullMatrix<double> expected_matrix(2,2);

  auto boundary = formulation::scalar::CFEM_Diffusion<2>::BoundaryType::kReflective;

  formulation::scalar::CFEM_Diffusion<2> test_diffusion(fe_mock_ptr,
                                                        cross_sections_ptr);

  auto init_token = test_diffusion.Precalculate(cell_ptr_);
  test_diffusion.FillBoundaryTerm(test_matrix, init_token, cell_ptr_, 0, boundary);

  EXPECT_TRUE(CompareMatrices(expected_matrix, test_matrix));
}

TEST_F(FormulationCFEMDiffusionTest, FillBoundaryTermTestVacuum) {
  dealii::FullMatrix<double> test_matrix(2,2);

  auto boundary = formulation::scalar::CFEM_Diffusion<2>::BoundaryType::kVacuum;

  std::array<double, 4> expected_values{3, 6,
                                        6, 13.5};
  dealii::FullMatrix<double> expected_matrix(2, 2, expected_values.begin());

  formulation::scalar::CFEM_Diffusion<2> test_diffusion(fe_mock_ptr,
                                                        cross_sections_ptr);

  auto init_token = test_diffusion.Precalculate(cell_ptr_);

  EXPECT_CALL(*fe_mock_ptr, SetFace(cell_ptr_, 0))
      .Times(1);
  EXPECT_CALL(*fe_mock_ptr, Jacobian(_))
      .Times(2)
      .WillRepeatedly(DoDefault());

  test_diffusion.FillBoundaryTerm(test_matrix, init_token, cell_ptr_, 0, boundary);

  EXPECT_TRUE(CompareMatrices(expected_matrix, test_matrix));

}

TEST_F(FormulationCFEMDiffusionTest, FillCellFixedSource) {
  std::vector<double> expected_values{6, 15};

  dealii::Vector<double> expected_vector{expected_values.begin(),
                                         expected_values.end()};
  dealii::Vector<double> test_vector(2);

  formulation::scalar::CFEM_Diffusion<2> test_diffusion(fe_mock_ptr,
                                                        cross_sections_ptr);

  EXPECT_CALL(*fe_mock_ptr, SetCell(_))
      .Times(1);
  EXPECT_CALL(*fe_mock_ptr, Jacobian(_))
      .Times(2)
      .WillRepeatedly(DoDefault());
  EXPECT_CALL(*fe_mock_ptr, ShapeValue(_,_))
      .Times(4)
      .WillRepeatedly(DoDefault());

  test_diffusion.FillCellFixedSource(test_vector, cell_ptr_, 0);

  EXPECT_TRUE(CompareVector(expected_vector, test_vector));

}

TEST_F(FormulationCFEMDiffusionTest, FillFissionSourceTest) {

  formulation::scalar::CFEM_Diffusion<2> test_diffusion(fe_mock_ptr,
                                                        cross_sections_ptr);

  dealii::Vector<double> test_vector(2);
  int material_id = 0;
  int group = 0;
  double k_effective = 1.05;
  // Make in-group moment
  std::vector<double> in_group_moment_values{0.5, 0.5};
  data::MomentVector in_group_moment(in_group_moment_values.begin(),
                                     in_group_moment_values.end());

  /* Make out-group moments (specifically in-group values in this are different
   * This is a somewhat tortured process due to the way dealii::Vectors are
   * defined (data::MomentVector is an alias) */
  std::vector<double> group_0_moment_values{0.75, 0.75};
  std::vector<double> group_1_moment_values{1.0, 1.0};
  data::MomentVector group_0_moment{group_0_moment_values.begin(),
                                    group_0_moment_values.end()};
  data::MomentVector group_1_moment{group_1_moment_values.begin(),
                                    group_1_moment_values.end()};
  data::MomentsMap out_group_moments;
  out_group_moments[{0,0,0}] = group_0_moment;
  out_group_moments[{1,0,0}] = group_1_moment;

  // Expected answer
  std::vector<double> expected_values{3.57142857142,
                                      8.92857142855};
  dealii::Vector<double> expected_vector{expected_values.begin(),
                                         expected_values.end()};

  EXPECT_CALL(*fe_mock_ptr, SetCell(_))
      .Times(1);
  EXPECT_CALL(*fe_mock_ptr, ValueAtQuadrature(group_1_moment))
      .WillOnce(Return(group_1_moment_values));
  EXPECT_CALL(*fe_mock_ptr, ValueAtQuadrature(in_group_moment))
      .WillOnce(Return(in_group_moment_values));

  test_diffusion.FillCellFissionSource(test_vector, cell_ptr_, group,
                                       k_effective, in_group_moment,
                                       out_group_moments);

  EXPECT_TRUE(CompareVector(expected_vector, test_vector));

}

TEST_F(FormulationCFEMDiffusionTest, FillScatteringSourceTest) {

  formulation::scalar::CFEM_Diffusion<2> test_diffusion(fe_mock_ptr,
                                                        cross_sections_ptr);

  dealii::Vector<double> test_vector(2);
  int material_id = 0;
  int group = 0;
  // Make in-group moment
  std::vector<double> in_group_moment_values{0.5, 0.5};
  data::MomentVector in_group_moment(in_group_moment_values.begin(),
                                     in_group_moment_values.end());

  /* Make out-group moments (specifically in-group values in this are different
   * This is a somewhat tortured process due to the way dealii::Vectors are
   * defined (data::MomentVector is an alias) */
  std::vector<double> group_0_moment_values{0.75, 0.75};
  std::vector<double> group_1_moment_values{1.0, 1.0};
  data::MomentVector group_0_moment{group_0_moment_values.begin(),
                                    group_0_moment_values.end()};
  data::MomentVector group_1_moment{group_1_moment_values.begin(),
                                    group_1_moment_values.end()};
  data::MomentsMap out_group_moments;
  out_group_moments[{0,0,0}] = group_0_moment;
  out_group_moments[{1,0,0}] = group_1_moment;

  // Expected answer
  std::vector<double> expected_values{3.75,
                                      9.375};
  dealii::Vector<double> expected_vector{expected_values.begin(),
                                         expected_values.end()};

  EXPECT_CALL(*fe_mock_ptr, SetCell(_))
      .Times(1);
  EXPECT_CALL(*fe_mock_ptr, ValueAtQuadrature(group_1_moment))
      .WillOnce(Return(group_1_moment_values));
  EXPECT_CALL(*fe_mock_ptr, ValueAtQuadrature(in_group_moment))
      .WillOnce(Return(in_group_moment_values));

  test_diffusion.FillCellScatteringSource(test_vector, cell_ptr_, group,
                                       in_group_moment,
                                       out_group_moments);

  EXPECT_TRUE(CompareVector(expected_vector, test_vector));

}


} // namespace