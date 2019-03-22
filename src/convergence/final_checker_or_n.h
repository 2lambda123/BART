#ifndef BART_SRC_CONVERGENCE_MOMENTS_FINAL_OR_N_H_
#define BART_SRC_CONVERGENCE_MOMENTS_FINAL_OR_N_H_

#include "convergence/final.h"
#include "convergence/status.h"

namespace bart {

namespace convergence {

template <typename CheckerType>
class FinalOrN : public Final{
 public:
  FinalOrN() = default;
  ~FinalOrN() = default;

  Status CheckFinalConvergence() override;

 protected:

};

} // namespace convergence

} // namespace bart

#endif // BART_SRC_CONVERGENCE_MOMENTS_FINAL_OR_N_H_