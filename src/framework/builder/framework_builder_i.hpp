#ifndef BART_SRC_FRAMEWORK_BUILDER_FRAMEWORK_BUILDER_I_HPP_
#define BART_SRC_FRAMEWORK_BUILDER_FRAMEWORK_BUILDER_I_HPP_

#include <memory>
#include <string>

#include "domain/definition_i.h"
#include "domain/finite_element/finite_element_i.h"
#include "framework/framework_i.hpp"
#include "framework/framework_parameters.hpp"
#include "formulation/stamper_i.h"
#include "quadrature/quadrature_set_i.h"
#include "problem/parameter_types.h"

namespace bart::framework::builder {

template <int dim>
class FrameworkBuilderI {
 public:
  using Domain = typename domain::DefinitionI<dim>;
  using FiniteElement = typename domain::finite_element::FiniteElementI<dim>;
  using FrameworkI = framework::FrameworkI;
  using QuadratureSet = typename quadrature::QuadratureSetI<dim>;
  using Stamper = formulation::StamperI<dim>;

  virtual ~FrameworkBuilderI() = default;
  virtual auto BuildDomain(const FrameworkParameters::DomainSize,
                           const FrameworkParameters::NumberOfCells,
                           const std::shared_ptr<FiniteElement>&,
                           const std::string material_mapping) -> std::unique_ptr<Domain> = 0;
  virtual auto BuildFiniteElement(
      const problem::CellFiniteElementType,
      const problem::DiscretizationType,
      const FrameworkParameters::PolynomialDegree) -> std::unique_ptr<FiniteElement> = 0;
  virtual auto BuildQuadratureSet(
      const problem::AngularQuadType,
      const FrameworkParameters::AngularQuadratureOrder) -> std::shared_ptr<QuadratureSet> = 0;
  virtual auto BuildStamper(const std::shared_ptr<Domain>&) -> std::unique_ptr<Stamper> = 0;
};

template <int dim>
auto BuildFramework(FrameworkBuilderI<dim>&,
                    const framework::FrameworkParameters&) -> std::unique_ptr<framework::FrameworkI>;

} // namespace bart::framework::builder

#endif //BART_SRC_FRAMEWORK_BUILDER_FRAMEWORK_BUILDER_I_HPP_
