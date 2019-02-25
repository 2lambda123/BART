#ifndef BART_SRC_CONVERGENCE_FLUX_CHECKER_I_H_
#define BART_SRC_CONVERGENCE_FLUX_CHECKER_I_H_

#include "../data/vector_parameters.h"

namespace bart {

namespace convergence {

/*! \brief Checks for convergence between two provided fluxes. */

class FluxCheckerI {
 public:
  virtual ~FluxCheckerI() = default;
  virtual bool isConverged(data::Flux &, data::Flux &) = 0;
  virtual bool isConverged() const = 0;
};
  
} // namespace convergence

} // namespace bart

#endif // BART_SRC_CONVERGENCE_FLUX_CHECKER_I_H_
