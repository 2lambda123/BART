#ifndef BART_SRC_FORMULATION_SCALAR_DRIFT_DIFFUSION_HPP_
#define BART_SRC_FORMULATION_SCALAR_DRIFT_DIFFUSION_HPP_

#include "formulation/scalar/drift_diffusion_i.hpp"

#include <memory>

#include "quadrature/calculators/angular_flux_integrator_i.hpp"
#include "calculator/drift_diffusion/drift_diffusion_vector_calculator_i.hpp"
#include "data/cross_sections/material_cross_sections.hpp"
#include "domain/finite_element/finite_element_i.hpp"
#include "utility/has_dependencies.h"

namespace bart::formulation::scalar {

template <int, typename ...> class DriftDiffusionIFactory;

/*! \brief Default implementation of the drift diffusion formulation.
  * @tparam dim spatial dimension.
 */
template <int dim>
class DriftDiffusion : public DriftDiffusionI<dim>, public utility::HasDependencies {
 public:
  using AngularFluxIntegrator = quadrature::calculators::AngularFluxIntegratorI;
  using CrossSections = data::cross_sections::CrossSectionsI;
  using DriftDiffusionCalculator = typename calculator::drift_diffusion::DriftDiffusionVectorCalculatorI<dim>;
  using FiniteElement = typename domain::finite_element::FiniteElementI<dim>;
  using typename DriftDiffusionI<dim>::Matrix;
  using typename DriftDiffusionI<dim>::CellPtr;
  using typename DriftDiffusionI<dim>::Vector;
  using typename DriftDiffusionI<dim>::VectorMap;

  using Factory = DriftDiffusionIFactory<dim, std::shared_ptr<FiniteElement>, std::shared_ptr<CrossSections>, std::shared_ptr<DriftDiffusionCalculator>, std::shared_ptr<AngularFluxIntegrator>>;

  /*! \brief Constructor. */
  DriftDiffusion(std::shared_ptr<FiniteElement>, std::shared_ptr<CrossSections>,
                 std::shared_ptr<DriftDiffusionCalculator>, std::shared_ptr<AngularFluxIntegrator>);

  auto FillCellBoundaryTerm(Matrix& to_fill, const CellPtr&, domain::FaceIndex, BoundaryType,
                            const VectorMap& group_angular_flux) const -> void override;

  auto FillCellDriftDiffusionTerm(Matrix &to_fill, const CellPtr&, system::EnergyGroup,
                                  const Vector& group_scalar_flux,
                                  const std::array<Vector, dim>& current) const -> void override;
  /*! \brief Access the angular flux integrator dependency. */
  [[nodiscard]] auto angular_flux_integrator_ptr() const { return angular_flux_integrator_ptr_.get(); }
  /*! \brief Access the finite element dependency. */
  [[nodiscard]] auto finite_element_ptr() const { return finite_element_ptr_.get(); }
  /*! \brief Access the cross-sections dependency. */
  [[nodiscard]] auto cross_sections_ptr() const { return cross_sections_ptr_.get(); }
  /*! \brief Access the drift-diffusion calculator dependency. */
  [[nodiscard]] auto drift_diffusion_calculator_ptr() const { return drift_diffusion_calculator_ptr_.get(); }
 protected:
  std::shared_ptr<FiniteElement> finite_element_ptr_{ nullptr };
  std::shared_ptr<CrossSections> cross_sections_ptr_{ nullptr };
  std::shared_ptr<DriftDiffusionCalculator> drift_diffusion_calculator_ptr_{ nullptr };
  std::shared_ptr<AngularFluxIntegrator> angular_flux_integrator_ptr_{ nullptr };
  int cell_quadrature_points_{ 0 };
  int cell_degrees_of_freedom_{ 0 };
  int face_quadrature_points_{ 0 };
 private:
  static bool is_registered_;
};

} // namespace bart::formulation::scalar

#endif //BART_SRC_FORMULATION_SCALAR_DRIFT_DIFFUSION_HPP_
