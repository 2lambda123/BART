#ifndef BART_SRC_DATA_FINITE_ELEMENT_H_
#define BART_SRC_DATA_FINITE_ELEMENT_H_

#include <memory>

#include <deal.II/fe/fe_values.h>

#include "../problem/parameter_types.h"

namespace bart {

namespace data {

template <int dim>
class FiniteElement {
 public:
  using DiscretizationType = bart::problem::DiscretizationType;
  FiniteElement(DiscretizationType discretization,
                int polynomial_degree);
  ~FiniteElement() = default;

  int polynomial_degree() { return polynomial_degree_; };
  dealii::FiniteElement<dim, dim> *finite_element() {
    return finite_element_.get(); };

 private:
  const int polynomial_degree_;
  std::shared_ptr<dealii::FiniteElement<dim, dim>> finite_element_;

  std::shared_ptr<dealii::FiniteElement<dim, dim>>
  GetFiniteElement(DiscretizationType discretization);
};

} // namespace data

} // namespace bart 

#endif // BART_SRC_DATA_FINITE_ELEMENT_H_
