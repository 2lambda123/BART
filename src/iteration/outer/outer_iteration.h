#ifndef BART_SRC_ITERATION_OUTER_OUTER_ITERATION_H_
#define BART_SRC_ITERATION_OUTER_OUTER_ITERATION_H_

#include <memory>

#include "convergence/final_i.h"
#include "convergence/reporter/mpi_i.h"
#include "iteration/group/group_solve_iteration_i.h"
#include "iteration/outer/outer_iteration_i.h"
#include "system/system.h"

namespace bart {

namespace iteration {

namespace outer {

template <typename ConvergenceType>
class OuterIteration : public OuterIterationI {
 public:
  using GroupIterator = iteration::group::GroupSolveIterationI;
  using ConvergenceChecker = convergence::FinalI<ConvergenceType>;
  using Reporter = convergence::reporter::MpiI;

  OuterIteration(
      std::unique_ptr<GroupIterator> group_iterator_ptr,
      std::unique_ptr<ConvergenceChecker> convergence_checker_ptr,
      const std::shared_ptr<Reporter> &reporter_ptr = nullptr);
  virtual ~OuterIteration() = default;
  virtual void IterateToConvergence(system::System &system);

  GroupIterator* group_iterator_ptr() const {
    return group_iterator_ptr_.get();
  }

  ConvergenceChecker* convergence_checker_ptr() const {
    return convergence_checker_ptr_.get();
  }

  Reporter* reporter_ptr() const {
    return reporter_ptr_.get();
  }

  std::vector<double> iteration_error() const override {
    return iteration_error_;
  }

 protected:
  virtual void InnerIterationToConvergence(system::System &system);
  virtual convergence::Status CheckConvergence(system::System &system) = 0;
  virtual void UpdateSystem(system::System& system, const int group,
                            const int angle) = 0;

  std::unique_ptr<GroupIterator> group_iterator_ptr_ = nullptr;
  std::unique_ptr<ConvergenceChecker> convergence_checker_ptr_ = nullptr;
  std::shared_ptr<Reporter> reporter_ptr_ = nullptr;
  std::vector<double> iteration_error_{};
};

} // namespace outer

} // namespace iteration

} // namespace bart

#endif //BART_SRC_ITERATION_OUTER_OUTER_ITERATION_H_
