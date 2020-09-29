#ifndef BART_SRC_TEST_HELPERS_MPI_TEST_FIXTURE_H_
#define BART_SRC_TEST_HELPERS_MPI_TEST_FIXTURE_H_

#include <deal.II/base/mpi.h>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_renumbering.h>
#include <deal.II/distributed/tria.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/grid/tria.h>
#include <deal.II/lac/dynamic_sparsity_pattern.h>
#include <deal.II/lac/petsc_sparse_matrix.h>

#include <gtest/gtest.h>

namespace bart {

namespace testing {

template <int dim>
struct TriangulationType {
  using type = dealii::parallel::distributed::Triangulation<dim>;
};

template <>
struct TriangulationType<1> {
  using type = dealii::Triangulation<1>;
};

template struct TriangulationType<2>;
template struct TriangulationType<3>;

/*! \brief Provides a configured dealii domain of a given dimension for testing.
 *
 * This class provides a fully set-up MPI dealii domain for running tests that
 * need it. This includes anything that needs to iterate over real cells or use
 * matrices configured with real sparsity patterns. Generally, tests should run
 * in all three dimensions. This requires a decent amount of google test
 * configuration knowledge and templating. A sample test set up in this way is
 * provided below.
 *
 * Google test allows you to make a templated test class and run tests using
 * multiple _types_ for that template. The dimensionality of the dealii domain
 * is defined using a templated integer, so we will use this feature. The
 * template must be a _type_ not a _value_ so we will use a special type called
 * an std::integral_constant. Integral constants for 1, 2, and 3 are provided
 * in bart::testing::AllDimensions. The dimension is then stored in the type
 * DimensionWrapper, and the actual dimension value is retrieved using
 * DimensionWrapper::value.
 *
 * \code{.cpp}

#include "path/to/tested_class.h"

#include "test_helpers/dealii_test_domain.h"
#include "test_helpers/gmock_wrapper.h"

namespace  {

using namespace bart;

template <typename DimensionWrapper> // type that wraps the dimension
class TestedClassTest :
    public ::testing::Test,
    // derive from the correct dimensioned dealii test domain
    public bart::testing::DealiiTestDomain<DimensionWrapper::value> {
 protected:
  static constexpr int dim = DimensionWrapper::value; // get the integer dimension

  void SetUp() override;
};

template <typename DimensionWrapper>
void TestedClassTest<DimensionWrapper>::SetUp() {
  this->SetUpDealii(); // Set up the dealii domain
}

TYPED_TEST_CASE(TestedClassTest, bart::testing::AllDimensions);

TYPED_TEST(TestedClassTest, Dummy) {
  EXPECT_TRUE(true);
}

} // namespace
 *
 * \endcode
 *
 *
 * @tparam dim spatial dimension.
 */
template <int dim>
 class DealiiTestDomain {
  public:
   static constexpr int dimension = dim;
   DealiiTestDomain(const double domain_size = 1.0, const int refinements = 2);
   DealiiTestDomain(const double domain_min, const double domain_max,
                    const int refinements);
   void SetUpDealii();
   void StampMatrix(dealii::PETScWrappers::MPI::SparseMatrix& to_stamp,
                    double value = 1);

   using Cell = typename dealii::DoFHandler<dim>::active_cell_iterator;
   dealii::AffineConstraints<double> constraint_matrix_;
   typename TriangulationType<dim>::type triangulation_;
   dealii::DoFHandler<dim> dof_handler_;
   dealii::FE_Q<dim> fe_;
   dealii::IndexSet locally_relevant_dofs;
   dealii::IndexSet locally_owned_dofs_;
   std::vector<Cell> cells_;
   dealii::DynamicSparsityPattern dsp_;

   dealii::PETScWrappers::MPI::SparseMatrix matrix_1, matrix_2, matrix_3;
   dealii::PETScWrappers::MPI::Vector vector_1, vector_2, vector_3;

  private:
   void SetUpDofs();
   const double domain_min_;
   const double domain_max_;
   const int refinements_;
};

template <int dim>
inline DealiiTestDomain<dim>::DealiiTestDomain(
    const double domain_min,
    const double domain_max,
    const int refinements)
    : triangulation_(MPI_COMM_WORLD,
                     typename dealii::Triangulation<dim>::MeshSmoothing(
                         dealii::Triangulation<dim>::smoothing_on_refinement |
                             dealii::Triangulation<dim>::smoothing_on_coarsening)),
      dof_handler_(triangulation_),
      fe_(1),
      domain_min_(domain_min),
      domain_max_(domain_max),
      refinements_(refinements) {}

template <int dim>
inline DealiiTestDomain<dim>::DealiiTestDomain(const double domain_size,
                                               const int refinements)
    : DealiiTestDomain(0, domain_size, refinements) {}

template <>
inline DealiiTestDomain<1>::DealiiTestDomain(const double domain_min,
                                             const double domain_max,
                                             const int refinements)
    : triangulation_(typename dealii::Triangulation<1>::MeshSmoothing(
    dealii::Triangulation<1>::smoothing_on_refinement |
        dealii::Triangulation<1>::smoothing_on_coarsening)),
      dof_handler_(triangulation_),
      fe_(1),
      domain_min_(domain_min),
      domain_max_(domain_max),
      refinements_(refinements) {}

template <>
inline DealiiTestDomain<1>::DealiiTestDomain(const double domain_size,
                                             const int refinements)
    : DealiiTestDomain(0, domain_size, refinements) {}

template <int dim>
inline void DealiiTestDomain<dim>::SetUpDealii() {
  dealii::GridGenerator::hyper_cube(triangulation_, this->domain_min_,
                                    this->domain_max_);

  triangulation_.refine_global(refinements_);

  SetUpDofs();

  for (auto cell = dof_handler_.begin_active(); cell != dof_handler_.end(); ++ cell) {
    if (cell->is_locally_owned())
      cells_.push_back(cell);
  }

  matrix_1.reinit(locally_owned_dofs_, locally_owned_dofs_, dsp_, MPI_COMM_WORLD);
  matrix_2.reinit(locally_owned_dofs_, locally_owned_dofs_, dsp_, MPI_COMM_WORLD);
  matrix_3.reinit(locally_owned_dofs_, locally_owned_dofs_, dsp_, MPI_COMM_WORLD);
  vector_1.reinit(locally_owned_dofs_, MPI_COMM_WORLD);
  vector_2.reinit(locally_owned_dofs_, MPI_COMM_WORLD);
  vector_3.reinit(locally_owned_dofs_, MPI_COMM_WORLD);
}

template <int dim>
inline void DealiiTestDomain<dim>::SetUpDofs() {
  dof_handler_.distribute_dofs(fe_);
  locally_owned_dofs_ = dof_handler_.locally_owned_dofs();
  dealii::DoFTools::extract_locally_relevant_dofs(dof_handler_,
                                                  locally_relevant_dofs);

  constraint_matrix_.clear();
  constraint_matrix_.reinit(locally_relevant_dofs);
  dealii::DoFTools::make_hanging_node_constraints(dof_handler_,
                                                  constraint_matrix_);
  constraint_matrix_.close();

  dsp_.reinit(locally_relevant_dofs.size(),
              locally_relevant_dofs.size(),
              locally_relevant_dofs);
  dealii::DoFTools::make_sparsity_pattern(dof_handler_, dsp_,
                                          constraint_matrix_, false);

  dealii::SparsityTools::distribute_sparsity_pattern(
      dsp_,
      dof_handler_.n_locally_owned_dofs_per_processor(),
      MPI_COMM_WORLD, locally_relevant_dofs);

  constraint_matrix_.condense(dsp_);
}

template <>
inline void DealiiTestDomain<1>::SetUpDofs() {
  auto n_mpi_processes = dealii::Utilities::MPI::n_mpi_processes(MPI_COMM_WORLD);
  auto this_process = dealii::Utilities::MPI::this_mpi_process(MPI_COMM_WORLD);

  dealii::GridTools::partition_triangulation(n_mpi_processes, triangulation_);
  dof_handler_.distribute_dofs(fe_);
  dealii::DoFRenumbering::subdomain_wise(dof_handler_);

  auto locally_owned_dofs_vector =
      dealii::DoFTools::locally_owned_dofs_per_subdomain(dof_handler_);

  locally_owned_dofs_ = locally_owned_dofs_vector.at(this_process);

  constraint_matrix_.clear();
  dealii::DoFTools::make_hanging_node_constraints(dof_handler_,
                                                  constraint_matrix_);
  constraint_matrix_.close();

  dsp_.reinit(dof_handler_.n_dofs(), dof_handler_.n_dofs());
  dealii::DoFTools::make_sparsity_pattern(dof_handler_, dsp_,
                                          constraint_matrix_, false);
}

// Stamps a matrix with a given value or 1
template <int dim>
inline void DealiiTestDomain<dim>::StampMatrix(
    dealii::PETScWrappers::MPI::SparseMatrix &to_stamp,
    const double value) {

  dealii::FullMatrix<double> cell_matrix(fe_.dofs_per_cell, fe_.dofs_per_cell);

  for (unsigned int i = 0; i < cell_matrix.m(); ++i) {
    for (unsigned int j = 0; j < cell_matrix.n(); ++j) {
      cell_matrix(i,j) = value;
    }
  }

  for (const auto& cell : cells_) {
    std::vector<dealii::types::global_dof_index> local_dof_indices(fe_.dofs_per_cell);
    cell->get_dof_indices(local_dof_indices);
    to_stamp.add(local_dof_indices, cell_matrix);
  }

  to_stamp.compress(dealii::VectorOperation::add);
}

using DealiiTestDomains = ::testing::Types<bart::testing::DealiiTestDomain<1>,
                                            bart::testing::DealiiTestDomain<2>,
                                            bart::testing::DealiiTestDomain<3>>;

} // namespace testing

} // namespace bart

#endif // BART_SRC_TEST_HELPERS_MPI_TEST_FIXTURE_H_
