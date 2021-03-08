#ifndef BART_SRC_FORMULATION_SCALAR_TESTS_DIFFUSION_MOCK_HPP_
#define BART_SRC_FORMULATION_SCALAR_TESTS_DIFFUSION_MOCK_HPP_

#include <deal.II/lac/full_matrix.h>
#include <deal.II/dofs/dof_accessor.h>

#include "system/moments/spherical_harmonic_types.h"
#include "formulation/scalar/diffusion_i.hpp"
#include "test_helpers/gmock_wrapper.h"

namespace bart::formulation::scalar {

template <int dim>
class DiffusionMock : public DiffusionI<dim> {
 public:
  using typename DiffusionI<dim>::BoundaryType;
  using typename DiffusionI<dim>::CellPtr;
  using typename DiffusionI<dim>::Matrix;
  using typename DiffusionI<dim>::Vector;
  using typename DiffusionI<dim>::GroupNumber;
  using typename DiffusionI<dim>::FaceNumber;

  MOCK_METHOD(void, Precalculate, (const CellPtr& cell_ptr), (override));
  MOCK_METHOD(void, FillCellStreamingTerm, (Matrix&, const CellPtr&, GroupNumber), (const, override));
  MOCK_METHOD(void, FillCellCollisionTerm, (Matrix&, const CellPtr&, GroupNumber), (const, override));
  MOCK_METHOD(void, FillBoundaryTerm, (Matrix&, const CellPtr&, const FaceNumber, BoundaryType), (const, override));
  MOCK_METHOD(void, FillCellFixedSource, (Vector& to_fill, const CellPtr&, GroupNumber), (const, override));
  MOCK_METHOD(void, FillCellFissionSource, (Vector&, const CellPtr&, GroupNumber, double,
      const system::moments::MomentVector&, const system::moments::MomentsMap&), (const, override));
  MOCK_METHOD(void, FillCellScatteringSource, (Vector&, const CellPtr&, GroupNumber,
      const system::moments::MomentsMap&), (const, override));
  MOCK_METHOD(bool, is_initialized, (), (const, override));
};

} // namespace bart::formulation::scalar

#endif //BART_SRC_FORMULATION_SCALAR_TESTS_DIFFUSION_MOCK_HPP_