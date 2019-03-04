#include "single_checker_l1_norm.h"

namespace bart {

namespace convergence {

namespace flux {

bool SingleCheckerL1Norm::CheckIfConverged(data::FluxVector &current_iteration,
                                           data::FluxVector &previous_iteration) {
  data::FluxVector difference{current_iteration};
  difference -= previous_iteration;
  delta_ = difference.l1_norm()/current_iteration.l1_norm();
  is_converged_ = delta_ <= max_delta_;
  return is_converged_;
}

} // namespace flux

} // namespace convergence

} // namespace bart
