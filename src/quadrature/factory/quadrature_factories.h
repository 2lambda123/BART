#ifndef BART_SRC_QUADRATURE_FACTORY_QUADRATURE_FACTORIES_H_
#define BART_SRC_QUADRATURE_FACTORY_QUADRATURE_FACTORIES_H_

#include <memory>

#include "quadrature/ordinate_i.h"
#include "quadrature/quadrature_generator_i.h"
#include "quadrature/quadrature_point_i.h"
#include "quadrature/quadrature_set_i.h"
#include "quadrature/quadrature_types.h"

namespace bart {

namespace quadrature {

namespace factory {

template <int dim>
std::shared_ptr<OrdinateI<dim>> MakeOrdinatePtr(
    const OrdinateType type = OrdinateType::kDefault);

template <int dim>
std::shared_ptr<QuadraturePointI<dim>> MakeQuadraturePointPtr(
    const QuadraturePointImpl impl = QuadraturePointImpl::kDefault);

template <int dim>
std::shared_ptr<QuadratureGeneratorI<dim>> MakeAngularQuadratureGeneratorPtr(
    const Order, const AngularQuadratureSetType);

template <int dim>
std::shared_ptr<QuadratureSetI<dim>> MakeQuadratureSetPtr(
    const QuadratureSetImpl impl = QuadratureSetImpl::kDefault);

template <int dim>
void FillQuadratureSet(
    QuadratureSetI<dim>*,
    const std::vector<std::pair<quadrature::CartesianPosition<dim>, quadrature::Weight>>&);


} // namespace factory

} // namespace quadrature

} // namespace bart

#endif //BART_SRC_QUADRATURE_FACTORY_QUADRATURE_FACTORIES_H_
