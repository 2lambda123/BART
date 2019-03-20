#ifndef BART_SRC_CONVERGENCE_MOMENTS_SINGLE_MOMENT_L1_NORM_H_
#define BART_SRC_CONVERGENCE_MOMENTS_SINGLE_MOMENT_L1_NORM_H_

#include "convergence/moments/single_moment_checker.h"

namespace bart {

namespace convergence {

namespace moments {

/*! \brief Checks for convergence between two fluxes using the percentage
 * change in the L1 norms */

class SingleMomentCheckerL1Norm : public SingleMomentChecker {
 public:
  /*! \brief Default constructor, setting max delta to \f$10^{-6}\f$. */
  SingleMomentCheckerL1Norm()
      : SingleMomentChecker(1e-6) {}

  explicit SingleMomentCheckerL1Norm(const double max_delta)
      : SingleMomentChecker(max_delta) {};

  ~SingleMomentCheckerL1Norm() = default;

  bool CheckIfConverged(
      const data::MomentVector &current_iteration,
      const data::MomentVector &previous_iteration) override;
};

} // namespace moments
  
} // namespace convergence

} // namespace bart

#endif // BART_SRC_CONVERGENCE_MOMENTS_SINGLE_MOMENT_L1_NORM_H_
