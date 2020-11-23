#ifndef BART_SRC_FRAMEWORK_BUILDER_FRAMEWORK_BUILDER_I_HPP_
#define BART_SRC_FRAMEWORK_BUILDER_FRAMEWORK_BUILDER_I_HPP_

#include <memory>
#include <string>

#include "domain/finite_element/finite_element_i.h"
#include "framework/framework_i.hpp"
#include "framework/framework_parameters.hpp"
#include "problem/parameter_types.h"

namespace bart::framework::builder {

template <int dim>
class FrameworkBuilderI {
 public:
  using FiniteElement = typename domain::finite_element::FiniteElementI<dim>;
  using FrameworkI = framework::FrameworkI;
  virtual auto BuildFiniteElement(
      const problem::CellFiniteElementType,
      const problem::DiscretizationType,
      const FrameworkParameters::PolynomialDegree) -> std::unique_ptr<FiniteElement> = 0;
};

template <int dim>
auto BuildFramework(FrameworkBuilderI<dim>&,
                    const framework::FrameworkParameters&) -> std::unique_ptr<framework::FrameworkI>;

} // namespace bart::framework::builder

#endif //BART_SRC_FRAMEWORK_BUILDER_FRAMEWORK_BUILDER_I_HPP_
