#include "formulation/cfem_diffusion_stamper.h"

#include <memory>

#include <deal.II/base/mpi.h>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/distributed/tria.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/lac/dynamic_sparsity_pattern.h>
#include <deal.II/lac/petsc_parallel_sparse_matrix.h>

#include "domain/tests/definition_mock.h"
#include "formulation/scalar/tests/cfem_diffusion_mock.h"
#include "test_helpers/gmock_wrapper.h"
#include "test_helpers/test_helper_functions.h"

namespace {

using ::testing::AssertionResult;
using ::testing::AssertionFailure, ::testing::AssertionSuccess;
using ::testing::DoDefault;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Unused;
using ::testing::_;

using namespace bart;
using Cell = domain::DefinitionI<2>::Cell;
using InitToken = formulation::scalar::CFEM_DiffusionI<2>::InitializationToken;
using Matrix = dealii::FullMatrix<double>;

class CFEMDiffusionStamperTest : public ::testing::Test {
 protected:
  std::unique_ptr<NiceMock<domain::DefinitionMock<2>>> mock_definition_ptr;
  std::unique_ptr<NiceMock<formulation::scalar::CFEM_DiffusionMock<2>>> mock_diffusion_ptr;
  void SetUp() override;
  InitToken init_token_;
};

// TODO(Josh) Move this to a header where other stamper tests can use it
void FillMatrixWithOnes(Matrix& to_fill, Unused, Unused, Unused, Unused) {
  for (unsigned int i = 0; i < to_fill.n_rows(); ++i) {
    for (unsigned int j = 0; j < to_fill.n_cols(); ++j) {
      to_fill(i,j) = 1;
    }
  }
}

void CFEMDiffusionStamperTest::SetUp() {
  mock_definition_ptr = std::make_unique<NiceMock<domain::DefinitionMock<2>>>();
  mock_diffusion_ptr =
      std::make_unique<NiceMock<formulation::scalar::CFEM_DiffusionMock<2>>>();

  ON_CALL(*mock_diffusion_ptr, Precalculate(_))
      .WillByDefault(Return(init_token_));
}

TEST_F(CFEMDiffusionStamperTest, Constructor) {
  Cell test_cell;
  std::vector<Cell> cells{test_cell};

  EXPECT_CALL(*mock_definition_ptr, Cells())
      .WillOnce(Return(cells));
  EXPECT_CALL(*mock_diffusion_ptr, Precalculate(_))
      .WillOnce(DoDefault());

  formulation::CFEM_DiffusionStamper<2> test_stamper(
      std::move(mock_diffusion_ptr),
      std::move(mock_definition_ptr));

  EXPECT_EQ(mock_diffusion_ptr, nullptr);
  EXPECT_EQ(mock_definition_ptr, nullptr);
}

// TODO(Josh) Put this in it's own header file?
class CFEMDiffusionStamperMPITests : public CFEMDiffusionStamperTest {
 protected:
  using Cell = typename dealii::DoFHandler<2>::active_cell_iterator;

  CFEMDiffusionStamperMPITests();

  void SetUp() override;
  void SetUpDealii();
  AssertionResult CompareMPIMatrices(
      const dealii::PETScWrappers::MPI::SparseMatrix& expected,
      const dealii::PETScWrappers::MPI::SparseMatrix& result);

  dealii::ConstraintMatrix constraint_matrix_;
  dealii::parallel::distributed::Triangulation<2> triangulation_;
  dealii::DoFHandler<2> dof_handler_;
  dealii::FE_Q<2> fe_;
  dealii::IndexSet locally_relevant_dofs;
  dealii::IndexSet locally_owned_dofs_;
  dealii::PETScWrappers::MPI::SparseMatrix system_matrix_, index_hits_;
  std::vector<Cell> cells_;
  std::vector<int> material_ids_;
};

AssertionResult CFEMDiffusionStamperMPITests::CompareMPIMatrices(
    const dealii::PETScWrappers::MPI::SparseMatrix& expected,
    const dealii::PETScWrappers::MPI::SparseMatrix& result) {

  for (const auto& cell : cells_) {
    std::vector<dealii::types::global_dof_index> local_dof_indices(fe_.dofs_per_cell);
    cell->get_dof_indices(local_dof_indices);
    for (const auto &index_i : local_dof_indices) {
      for (const auto &index_j : local_dof_indices) {
        if (result(index_i, index_j) != expected(index_i, index_j)) {
          return AssertionFailure() << "Entry (" << index_i << ", " << index_j <<
                                    ") has value: " << result(index_i, index_j) <<
                                    ", expected: " << expected(index_i, index_j);
        }
      }
    }
  }
  return AssertionSuccess();
}

CFEMDiffusionStamperMPITests::CFEMDiffusionStamperMPITests()
    : triangulation_(MPI_COMM_WORLD,
                     typename dealii::Triangulation<2>::MeshSmoothing(
                         dealii::Triangulation<2>::smoothing_on_refinement |
                             dealii::Triangulation<2>::smoothing_on_coarsening)),
      dof_handler_(triangulation_),
      fe_(1) {}

void CFEMDiffusionStamperMPITests::SetUp() {
  CFEMDiffusionStamperTest::SetUp();
  SetUpDealii();

  for (const auto& cell : cells_) {
    int mat_id = btest::RandomDouble(0, 10);
    material_ids_.push_back(mat_id);
    cell->set_material_id(mat_id);
  }

  ON_CALL(*mock_definition_ptr, Cells())
      .WillByDefault(Return(cells_));
  dealii::FullMatrix<double> cell_matrix(fe_.dofs_per_cell,
                                         fe_.dofs_per_cell);
  ON_CALL(*mock_definition_ptr, GetCellMatrix())
      .WillByDefault(Return(cell_matrix));
}

void CFEMDiffusionStamperMPITests::SetUpDealii() {
  // Create triangulation
  dealii::GridGenerator::hyper_cube(triangulation_, 0, 1);
  triangulation_.refine_global(2);
  // Distribute DOFS, get local index sets and cells
  dof_handler_.distribute_dofs(fe_);
  dealii::DoFTools::extract_locally_relevant_dofs(dof_handler_,
                                                  locally_relevant_dofs);
  locally_owned_dofs_ = dof_handler_.locally_owned_dofs();
  for (auto cell = dof_handler_.begin_active(); cell != dof_handler_.end(); ++ cell) {
    if (cell->is_locally_owned())
      cells_.push_back(cell);
  }

  // Make constraint matrix and DSP for matrices
  constraint_matrix_.clear();
  constraint_matrix_.reinit(locally_relevant_dofs);
  dealii::DoFTools::make_hanging_node_constraints(dof_handler_,
                                                  constraint_matrix_);
  constraint_matrix_.close();

  dealii::DynamicSparsityPattern dsp(locally_relevant_dofs);
  dealii::DoFTools::make_sparsity_pattern(dof_handler_, dsp,
                                          constraint_matrix_, false);
  dealii::SparsityTools::distribute_sparsity_pattern(
      dsp,
      dof_handler_.n_locally_owned_dofs_per_processor(),
      MPI_COMM_WORLD, locally_relevant_dofs);
  constraint_matrix_.condense(dsp);

  // Set up MPI matrices
  system_matrix_.reinit(locally_owned_dofs_, locally_owned_dofs_, dsp, MPI_COMM_WORLD);
  index_hits_.reinit(locally_owned_dofs_, locally_owned_dofs_, dsp, MPI_COMM_WORLD);

  std::vector<dealii::types::global_dof_index> local_dof_indices(fe_.dofs_per_cell);

  for (auto cell : cells_) {
    cell->get_dof_indices(local_dof_indices);
    for (auto index_i : local_dof_indices) {
      for (auto index_j : local_dof_indices) {
        index_hits_.add(index_i, index_j, 1);
      }
    }
    index_hits_.compress(dealii::VectorOperation::add);
  }
}


TEST_F(CFEMDiffusionStamperMPITests, StampStreaming) {

  int group_number = 1;

  for (auto const& cell : cells_) {
    EXPECT_CALL(*mock_diffusion_ptr,
        FillCellStreamingTerm(_, _, cell, cell->material_id(), group_number))
        .WillOnce(Invoke(FillMatrixWithOnes));
  }
  EXPECT_CALL(*mock_definition_ptr, GetCellMatrix())
      .WillOnce(DoDefault());

  formulation::CFEM_DiffusionStamper<2> test_stamper(
      std::move(mock_diffusion_ptr),
      std::move(mock_definition_ptr));

  test_stamper.StampStreamingTerm(system_matrix_, group_number);

  EXPECT_TRUE(CompareMPIMatrices(system_matrix_, index_hits_));
}





} // namespace