#ifndef BART_SRC_ITERATION_GROUP_GROUP_SOURCE_ITERATION_H_
#define BART_SRC_ITERATION_GROUP_GROUP_SOURCE_ITERATION_H_

#include "iteration/group/group_solve_iteration.h"
#include "formulation/updater/scattering_source_updater_i.h"
#include "formulation/updater/boundary_conditions_updater_i.h"

namespace bart {

namespace iteration {

namespace group {

template <int dim>
class GroupSourceIteration : public GroupSolveIteration<dim> {
 public:
  using typename GroupSolveIteration<dim>::GroupSolver;
  using typename GroupSolveIteration<dim>::ConvergenceChecker;
  using typename GroupSolveIteration<dim>::MomentCalculator;
  using typename GroupSolveIteration<dim>::MomentMapConvergenceChecker;
  using typename GroupSolveIteration<dim>::GroupSolution;
  using typename GroupSolveIteration<dim>::Reporter;

  using SourceUpdater = formulation::updater::ScatteringSourceUpdaterI;
  using BoundaryConditionsUpdater = formulation::updater::BoundaryConditionsUpdaterI;

  GroupSourceIteration(
      std::unique_ptr<GroupSolver> group_solver_ptr,
      std::unique_ptr<ConvergenceChecker> convergence_checker_ptr,
      std::unique_ptr<MomentCalculator> moment_calculator_ptr,
      const std::shared_ptr<GroupSolution> &group_solution_ptr,
      const std::shared_ptr<SourceUpdater> &source_updater_ptr,
      const std::shared_ptr<Reporter> &reporter_ptr = nullptr,
      std::unique_ptr<MomentMapConvergenceChecker>
          moment_map_convergence_checker_ptr = nullptr);
  GroupSourceIteration(
      std::unique_ptr<GroupSolver> group_solver_ptr,
      std::unique_ptr<ConvergenceChecker> convergence_checker_ptr,
      std::unique_ptr<MomentCalculator> moment_calculator_ptr,
      const std::shared_ptr<GroupSolution> &group_solution_ptr,
      const std::shared_ptr<SourceUpdater> &source_updater_ptr,
      const std::shared_ptr<BoundaryConditionsUpdater>& boundary_condition_updater_ptr,
      const std::shared_ptr<Reporter> &reporter_ptr = nullptr,
      std::unique_ptr<MomentMapConvergenceChecker>
      moment_map_convergence_checker_ptr = nullptr);
  virtual ~GroupSourceIteration() = default;

  BoundaryConditionsUpdater* boundary_conditions_updater_ptr() const {
    return boundary_condition_updater_ptr_.get(); }
  SourceUpdater* source_updater_ptr() const { return source_updater_ptr_.get(); }

 protected:
  std::shared_ptr<BoundaryConditionsUpdater> boundary_condition_updater_ptr_;
  std::shared_ptr<SourceUpdater> source_updater_ptr_;
  void UpdateSystem(system::System &system, const int group,
                    const int angle) override;
  virtual void PerformPerGroup(system::System& system,
                               const int group) override;

};

} // namespace group

} // namespace iteration

} //namespace bart

#endif //BART_SRC_ITERATION_GROUP_GROUP_SOURCE_ITERATION_H_
