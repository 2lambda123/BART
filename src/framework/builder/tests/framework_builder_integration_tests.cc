
#include <deal.II/fe/fe_q.h>

#include "framework/builder/framework_builder.h"

// Instantiated concerete classes
#include "convergence/reporter/mpi_noisy.h"
#include "convergence/final_checker_or_n.h"
#include "convergence/parameters/single_parameter_checker.h"
#include "convergence/moments/single_moment_checker_i.h"
#include "data/cross_sections.h"
#include "domain/finite_element/finite_element_gaussian.h"
#include "domain/definition.h"
#include "formulation/angular/self_adjoint_angular_flux.h"
#include "quadrature/quadrature_set.h"
#include "solver/gmres.h"
#include "solver/group/single_group_solver.h"
#include "iteration/initializer/initialize_fixed_terms_once.h"

// Mock objects
#include "domain/finite_element/tests/finite_element_mock.h"
#include "material/tests/mock_material.h"
#include "problem/tests/parameters_mock.h"
#include "formulation/updater/tests/fixed_updater_mock.h"
#include "quadrature/tests/quadrature_set_mock.h"

#include "test_helpers/gmock_wrapper.h"
#include "test_helpers/test_helper_functions.h"

namespace {

using namespace bart;

using ::testing::Return, ::testing::NiceMock, ::testing::DoDefault;

template <typename DimensionWrapper>
class FrameworkBuilderIntegrationTest : public ::testing::Test {
 public:
  static constexpr int dim = DimensionWrapper::value;

  using FrameworkBuilder = framework::builder::FrameworkBuilder<dim>;
  using ProblemParameters = NiceMock<problem::ParametersMock>;
  using Material = NiceMock<btest::MockMaterial>;

  FrameworkBuilderIntegrationTest()
      : mock_material() {}

  FrameworkBuilder test_builder;
  ProblemParameters parameters;
  Material mock_material;

  // Test Parameters
  const int polynomial_degree = 2;
  std::vector<double> spatial_max;
  std::vector<int> n_cells;
  const int n_energy_groups = 3;
  std::array<int, 4> dofs_per_cell_by_dim_{1, 3, 9, 27};

  void SetUp() override;
};

template <typename DimensionWrapper>
void FrameworkBuilderIntegrationTest<DimensionWrapper>::SetUp() {
  for (int i = 0; i < this->dim; ++i) {
    spatial_max.push_back(10);
    n_cells.push_back(2);
  }

  std::map<problem::Boundary, bool> reflective_bcs{
      {problem::Boundary::kXMin, true},
      {problem::Boundary::kXMax, true},
      {problem::Boundary::kYMin, false},
      {problem::Boundary::kYMax, false},
      {problem::Boundary::kZMin, false},
      {problem::Boundary::kZMax, false},
  };

  ON_CALL(parameters, NEnergyGroups())
      .WillByDefault(Return(n_energy_groups));
  ON_CALL(parameters, NCells())
      .WillByDefault(Return(n_cells));
  ON_CALL(parameters, SpatialMax())
      .WillByDefault(Return(spatial_max));
  ON_CALL(parameters, FEPolynomialDegree())
      .WillByDefault(Return(polynomial_degree));
  ON_CALL(parameters, TransportModel())
      .WillByDefault(Return(problem::EquationType::kDiffusion));
  ON_CALL(parameters, ReflectiveBoundary())
      .WillByDefault(Return(reflective_bcs));
}

TYPED_TEST_CASE(FrameworkBuilderIntegrationTest,
                bart::testing::AllDimensions);

TYPED_TEST(FrameworkBuilderIntegrationTest, BuildConvergenceReporterTest) {
  using ExpectedType = convergence::reporter::MpiNoisy;

  auto convergence_reporter_ptr = this->test_builder.BuildConvergenceReporter();

  ASSERT_NE(nullptr,
            dynamic_cast<ExpectedType*>(convergence_reporter_ptr.get()));
}

TYPED_TEST(FrameworkBuilderIntegrationTest, BuildDomainTest) {
  constexpr int dim = this->dim;
  auto finite_element_ptr =
      std::make_shared<NiceMock<domain::finite_element::FiniteElementMock<dim>>>();

  EXPECT_CALL(this->parameters, NCells())
      .WillOnce(DoDefault());
  EXPECT_CALL(this->parameters, SpatialMax())
      .WillOnce(DoDefault());

  auto test_domain_ptr = this->test_builder.BuildDomain(
      this->parameters, finite_element_ptr, "1 1 2 2");

  EXPECT_NE(dynamic_cast<domain::Definition<this->dim>*>(test_domain_ptr.get()),
            nullptr);
}

TYPED_TEST(FrameworkBuilderIntegrationTest, BuildFiniteElementTest) {
  constexpr int dim = this->dim;
  EXPECT_CALL(this->parameters, FEPolynomialDegree())
      .WillOnce(DoDefault());

  using ExpectedType = domain::finite_element::FiniteElementGaussian<dim>;

  auto finite_element_ptr = this->test_builder.BuildFiniteElement(this->parameters);
  auto gaussian_ptr = dynamic_cast<ExpectedType*>(finite_element_ptr.get());

  EXPECT_NE(gaussian_ptr, nullptr);
  EXPECT_EQ(finite_element_ptr->polynomial_degree(), this->polynomial_degree);
  auto dealii_finite_element_ptr = dynamic_cast<dealii::FE_Q<dim>*>(
      finite_element_ptr->finite_element());
  EXPECT_NE(dealii_finite_element_ptr, nullptr);
}

TYPED_TEST(FrameworkBuilderIntegrationTest, BuildLSAngularQuadratureSet) {
  constexpr int dim = this->dim;
  const int order = 4;
  EXPECT_CALL(this->parameters, AngularQuad())
      .WillOnce(Return(problem::AngularQuadType::kLevelSymmetricGaussian));
  EXPECT_CALL(this->parameters, AngularQuadOrder())
      .WillOnce(Return(order));

  if (dim == 3) {
    using ExpectedType = quadrature::QuadratureSet<dim>;
    auto quadrature_set = this->test_builder.BuildQuadratureSet(this->parameters);
    ASSERT_NE(nullptr, quadrature_set);
    ASSERT_NE(nullptr, dynamic_cast<ExpectedType*>(quadrature_set.get()));
    EXPECT_EQ(quadrature_set->size(), order * (order + 2));
  } else {
    EXPECT_ANY_THROW({
      auto quadrature_set = this->test_builder.BuildQuadratureSet(this->parameters);
    });
  }
}

TYPED_TEST(FrameworkBuilderIntegrationTest, BuildSingleGroupSolver) {
  using ExpectedType = solver::group::SingleGroupSolver;

  auto solver_ptr = this->test_builder.BuildSingleGroupSolver(100, 1e-12);

  ASSERT_NE(nullptr, solver_ptr);

  auto dynamic_ptr = dynamic_cast<ExpectedType*>(solver_ptr.get());
  ASSERT_NE(nullptr, dynamic_ptr);

  using ExpectedLinearSolverType = solver::GMRES;

  auto linear_solver_ptr = dynamic_cast<ExpectedLinearSolverType*>(
      dynamic_ptr->linear_solver_ptr());

  ASSERT_NE(nullptr, linear_solver_ptr);
  EXPECT_EQ(linear_solver_ptr->convergence_tolerance(), 1e-12);
  EXPECT_EQ(linear_solver_ptr->max_iterations(), 100);
}

TYPED_TEST(FrameworkBuilderIntegrationTest, BuildConvergenceChecker) {
  const double max_delta = 1e-4;
  const int max_iterations = 100;

  auto convergence_ptr =
      this->test_builder.BuildParameterConvergenceChecker(
          max_delta,
          max_iterations);

  using ParameterConvergenceChecker = convergence::FinalCheckerOrN<double, convergence::parameters::SingleParameterChecker>;


  ASSERT_NE(convergence_ptr, nullptr);
  EXPECT_NE(nullptr,
            dynamic_cast<ParameterConvergenceChecker*>(convergence_ptr.get()));
  EXPECT_EQ(convergence_ptr->max_iterations(), max_iterations);

}

TYPED_TEST(FrameworkBuilderIntegrationTest, BuildMomentConvergenceChecker) {
  const double max_delta = 1e-4;
  const int max_iterations = 100;

  auto convergence_ptr =
      this->test_builder.BuildMomentConvergenceChecker(
          max_delta,
          max_iterations);

  using MomentConvergenceChecker =
  convergence::FinalCheckerOrN<system::moments::MomentVector,
                               convergence::moments::SingleMomentCheckerI>;


  ASSERT_NE(convergence_ptr, nullptr);
  EXPECT_NE(nullptr,
            dynamic_cast<MomentConvergenceChecker*>(convergence_ptr.get()));
  EXPECT_EQ(convergence_ptr->max_iterations(), max_iterations);

}

TYPED_TEST(FrameworkBuilderIntegrationTest, BuildSAAFFormulationTest) {
  constexpr int dim = this->dim;

  auto finite_element_ptr =
      std::make_shared<domain::finite_element::FiniteElementMock<dim>>();
  auto cross_sections_ptr =
      std::make_shared<data::CrossSections>(this->mock_material);
  auto quadrature_set_ptr =
      std::make_shared<quadrature::QuadratureSetMock<dim>>();

  EXPECT_CALL(*finite_element_ptr, dofs_per_cell());
  EXPECT_CALL(*finite_element_ptr, n_cell_quad_pts());
  EXPECT_CALL(*finite_element_ptr, n_face_quad_pts());

  auto saaf_formulation_ptr = this->test_builder.BuildSAAFFormulation(
      finite_element_ptr, cross_sections_ptr, quadrature_set_ptr);

  using ExpectedType = formulation::angular::SelfAdjointAngularFlux<dim>;

  ASSERT_NE(saaf_formulation_ptr, nullptr);
  EXPECT_NE(nullptr, dynamic_cast<ExpectedType*>(saaf_formulation_ptr.get()));
}

/* ===== Non-dimensional tests =================================================
 * These tests instantiate classes and use depdent classes that do not have a
 * dimension template varaible and therefore only need to be run in a single
 * dimension.
*/


class FrameworkBuilderIntegrationNonDimTest
 : public FrameworkBuilderIntegrationTest<bart::testing::OneD> {};

TEST_F(FrameworkBuilderIntegrationNonDimTest, BuildInitializer) {
  auto fixed_updater_ptr =
      std::make_shared<formulation::updater::FixedUpdaterMock>();

  using ExpectedType = iteration::initializer::InitializeFixedTermsOnce;
  const int total_groups = bart::test_helpers::RandomDouble(1, 10);
  const int total_angles = total_groups + 1;

  auto initializer_ptr = this->test_builder.BuildInitializer(fixed_updater_ptr,
                                                             total_groups,
                                                             total_angles);
  auto dynamic_ptr = dynamic_cast<ExpectedType*>(initializer_ptr.get());
  ASSERT_NE(initializer_ptr, nullptr);
  ASSERT_NE(dynamic_ptr, nullptr);
  EXPECT_EQ(dynamic_ptr->total_angles(), total_angles);
  EXPECT_EQ(dynamic_ptr->total_groups(), total_groups);
}


} // namespace
