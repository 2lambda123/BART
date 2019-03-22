#ifndef BART_SRC_CONVERGENCE_MOMENTS_FINAL_OR_N_H_
#define BART_SRC_CONVERGENCE_MOMENTS_FINAL_OR_N_H_

#include <memory>

#include "convergence/final.h"
#include "convergence/status.h"

namespace bart {

namespace convergence {

/*! \brief Check convergence via a checker class, or N iterations.
 *
 * \tparam CheckerType type of checker used to determine convergence.
 */

template <typename CompareType, typename CheckerType>
class FinalCheckerOrN : public Final<CompareType>{
 public:
  /*! \brief Constructor.
   *
   * \param checker_ptr pointer to the convergence checker that this class
   * will take ownership of.
   */
  explicit FinalCheckerOrN(std::unique_ptr<CheckerType> checker_ptr)
      : checker_ptr_(std::move(checker_ptr)) {}
  ~FinalCheckerOrN() = default;

  Status CheckFinalConvergence(CompareType& current_iteration,
                               CompareType& previous_iteration) override;

 protected:
  using Final<CompareType>::convergence_status_;
  std::unique_ptr<CheckerType> checker_ptr_;
};

} // namespace convergence

} // namespace bart

#endif // BART_SRC_CONVERGENCE_MOMENTS_FINAL_OR_N_H_