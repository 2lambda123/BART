#ifndef BART_SRC_CALCULATOR_CELL_FISSION_SOURCE_NORM_H_
#define BART_SRC_CALCULATOR_CELL_FISSION_SOURCE_NORM_H_

#include <memory>

#include "calculator/cell/fission_source_norm_i.h"
#include "data/cross_sections.h"
#include "domain/finite_element_i.h"

namespace bart {

namespace calculator {

namespace cell {

/*! \brief Calculates the cell norm of the fission source.
 *
 * @tparam dim problem spatial dimension
 */
template <int dim>
class FissionSourceNorm : public FissionSourceNormI {
 public:
  FissionSourceNorm(
      std::shared_ptr<domain::FiniteElementI<dim>> finite_element_ptr,
      std::shared_ptr<data::CrossSections> cross_sections_ptr)
      : finite_element_ptr_(finite_element_ptr),
        cross_sections_ptr_(cross_sections_ptr)
      {};
  ~FissionSourceNorm() = default;

 private:
  std::shared_ptr<domain::FiniteElementI<dim>> finite_element_ptr_;
  std::shared_ptr<data::CrossSections> cross_sections_ptr_;
};

} // namespace cell

} // namespace calculator

} // namespace bart

#endif // BART_SRC_CALCULATOR_CELL_FISSION_SOURCE_NORM_H_