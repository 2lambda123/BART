#ifndef BART_SRC_CONVERGENCE_GROUP_FLUX_CHECKER_SEQUENTIAL_H_
#define BART_SRC_CONVERGENCE_GROUP_FLUX_CHECKER_SEQUENTIAL_H_

#include <memory>

#include "flux_checker_i.h"
#include "group_flux_checker_i.h"


namespace bart {

namespace convergence {

/*! \brief Checks each flux group sequentially for convergence.
 * Will not re-check converged groups until final convergence is believed to
 * have been reached. */

class GroupFluxCheckerSequential : public GroupFluxCheckerI {
 public:
  GroupFluxCheckerSequential() = default;
  GroupFluxCheckerSequential(std::unique_ptr<FluxCheckerI> &tester);
  ~GroupFluxCheckerSequential() = default;
  
  void ProvideChecker(std::unique_ptr<FluxCheckerI> &tester) {
    tester_ = std::move(tester); };
  
  bool isConverged(data::GroupFluxPointers &current,
                   data::GroupFluxPointers &last);
  int GetFailedGroup() const { return failed_group_; };

 private:
  /*! Flux convergence tester that will be used to check sequentially */
  std::unique_ptr<FluxCheckerI> tester_;
  /*! Stores the ID of the first group that failed the convergence check */
  int failed_group_; 
};

} // namespace convergence

} // namespace bart

#endif // BART_SRC_CONVERGENCE_GROUP_FLUX_CHECKER_SEQUENTIAL_H_
