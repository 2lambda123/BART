#ifndef BART_SRC_ITERATION_GROUP_GROUP_SOLVE_ITERATION_H_
#define BART_SRC_ITERATION_GROUP_GROUP_SOLVE_ITERATION_H_

#include "convergence/final_i.h"
#include "iteration/group/group_solve_iteration_i.h"
#include "quadrature/calculators/spherical_harmonic_moments_i.h"

#include <memory>

#include "solver/group/single_group_solver_i.h"

namespace bart {

namespace iteration {

namespace group {

template <int dim>
class GroupSolveIteration : public GroupSolveIterationI {
 public:
  using GroupSolver = solver::group::SingleGroupSolverI;
  using ConvergenceChecker = convergence::FinalI<system::moments::MomentVector>;
  using MomentCalculator = quadrature::calculators::SphericalHarmonicMomentsI<dim>;

  GroupSolveIteration(
      std::unique_ptr<GroupSolver> group_solver_ptr,
      std::unique_ptr<ConvergenceChecker> convergence_checker_ptr,
      std::unique_ptr<MomentCalculator> moment_calculator_ptr);
  virtual ~GroupSolveIteration() = default;

  GroupSolver* group_solver() const {
    return group_solver_ptr_.get();
  }

  ConvergenceChecker* convergence_checker_ptr() const {
    return convergence_checker_ptr_.get();
  }

  MomentCalculator* moment_calculator_ptr() const {
    return moment_calculator_ptr_.get();
  }

 protected:
  std::unique_ptr<GroupSolver> group_solver_ptr_ = nullptr;
  std::unique_ptr<ConvergenceChecker> convergence_checker_ptr_ = nullptr;
  std::unique_ptr<MomentCalculator> moment_calculator_ptr_ = nullptr;
};

} // namespace group

} // namespace iteration



} //namespace bart

#endif //BART_SRC_ITERATION_GROUP_GROUP_SOLVE_ITERATION_H_
