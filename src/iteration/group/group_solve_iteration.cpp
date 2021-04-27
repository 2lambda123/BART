#include "iteration/group/group_solve_iteration.hpp"

namespace bart::iteration::group {

template <int dim>
GroupSolveIteration<dim>::GroupSolveIteration(
    std::unique_ptr<GroupSolver> group_solver_ptr,
    std::unique_ptr<ConvergenceChecker> convergence_checker_ptr,
    std::unique_ptr<MomentCalculator> moment_calculator_ptr,
    const std::shared_ptr<GroupSolution> group_solution_ptr,
    std::unique_ptr<MomentMapConvergenceChecker> moment_map_convergence_checker_ptr)
    : group_solver_ptr_(std::move(group_solver_ptr)),
      convergence_checker_ptr_(std::move(convergence_checker_ptr)),
      moment_calculator_ptr_(std::move(moment_calculator_ptr)),
      group_solution_ptr_(group_solution_ptr),
      moment_map_convergence_checker_ptr_(std::move(moment_map_convergence_checker_ptr)) {
  std::string function_name{ "GroupSolveIteration constructor" };
  this->AssertPointerNotNull(group_solver_ptr_.get(), "group solver", function_name);
  this->AssertPointerNotNull(convergence_checker_ptr_.get(), "convergence checker", function_name);
  this->AssertPointerNotNull(moment_calculator_ptr_.get(), "moment calculator", function_name);
  this->AssertPointerNotNull(group_solution_ptr_.get(), "group solution pointer", function_name);
}

template<int dim>
auto GroupSolveIteration<dim>::Iterate(System &system) -> void {
  const int total_groups{ system.total_groups };
  const int total_angles{ system.total_angles };
  MomentVector current_scalar_flux, previous_scalar_flux;
  system::moments::MomentsMap previous_moments_map;

  for (int group = 0; group < total_groups; ++group) {
    auto& current_moments = *system.current_moments;
    auto& previous_moments = *system.previous_moments;
    const int max_harmonic_l = current_moments.max_harmonic_l();
    for (int l = 0; l <= max_harmonic_l; ++l) {
      for (int m = -l; m <= l; ++m) {
        previous_moments[{group, l, m}] = current_moments[{group, l, m}];
      }
    }
  }
  data_ports::StatusPort::Expose("..Inner group iteration\n");
  moment_map_convergence_checker_ptr_->Reset();
  convergence::Status all_group_convergence_status;
  all_group_convergence_status.is_complete = true;
  do {
    previous_moments_map = system.current_moments->moments();

    for (int group = 0; group < total_groups; ++group) {
      PerformPerGroup(system, group);

      convergence::Status convergence_status;
      convergence_checker_ptr_->Reset();
      do {
        if (!convergence_status.is_complete) {
          for (int angle = 0; angle < total_angles; ++angle)
            UpdateSystem(system, group, angle);
        }

        previous_scalar_flux = current_scalar_flux;

        SolveGroup(group, system);

        current_scalar_flux = GetScalarFlux(group, system);

        if (convergence_status.iteration_number == 0) {
          previous_scalar_flux = current_scalar_flux;
          previous_scalar_flux = 0;
        }

        convergence_status = CheckConvergence(current_scalar_flux,
                                              previous_scalar_flux);

        data_ports::ConvergenceStatusPort::Expose(convergence_status);
        UpdateCurrentMoments(system, group);
      } while (!convergence_status.is_complete);

      if (is_storing_angular_solution_)
        StoreAngularSolution(system, group);
    }

    if (post_iteration_subroutine_ptr_ != nullptr) {
      data_ports::StatusPort::Expose("==== COMMENCE GROUP SOLVE POST ITERATION SUBROUTINE ==== \n");
      post_iteration_subroutine_ptr_->Execute(system);
      data_ports::StatusPort::Expose("==== COMPLETED GROUP SOLVE POST ITERATION SUBROUTINE  ==== \n");
    }

    if (moment_map_convergence_checker_ptr_ != nullptr) {
      all_group_convergence_status =
          moment_map_convergence_checker_ptr_->ConvergenceStatus(
              system.current_moments->moments(), previous_moments_map);
      data_ports::StatusPort::Expose("....All group convergence: ");
      data_ports::ConvergenceStatusPort::Expose(all_group_convergence_status);
    }
  } while(!all_group_convergence_status.is_complete);
  ExposeIterationData(system);
}

template <int dim>
auto GroupSolveIteration<dim>::SolveGroup(const int group, System &system) -> void {
  group_solver_ptr_->SolveGroup(group, system, *group_solution_ptr_);
}

template <int dim>
auto GroupSolveIteration<dim>::GetScalarFlux(const int group, System& /*system*/) -> MomentVector {
  return moment_calculator_ptr_->CalculateMoment(group_solution_ptr_.get(), group, 0, 0);
}

template <int dim>
auto GroupSolveIteration<dim>::CheckConvergence(const MomentVector &current_iteration,
                                                const MomentVector &previous_iteration) -> convergence::Status{
  return convergence_checker_ptr_->ConvergenceStatus(current_iteration, previous_iteration);
}

template <int dim>
auto GroupSolveIteration<dim>::UpdateCurrentMoments(System &system, const int group) -> void {
  auto& current_moments = *system.current_moments;
  const int max_harmonic_l = current_moments.max_harmonic_l();

  for (int l = 0; l <= max_harmonic_l; ++l) {
    for (int m = -l; m <= l; ++m) {
      current_moments[{group, l, m}] = moment_calculator_ptr_->CalculateMoment(group_solution_ptr_.get(), group, l, m);
    }
  }
}

template<int dim>
auto GroupSolveIteration<dim>::PerformPerGroup(System& /*system*/, const int group) -> void {
  std::string report{"....Group: "};
  report += std::to_string(group);
  report += "\n";
  data_ports::StatusPort::Expose(report);
}

template<int dim>
auto GroupSolveIteration<dim>::StoreAngularSolution(System& system, const int group) -> void {
  for (int angle = 0; angle < system.total_angles; ++angle) {
    auto& stored_solution = angular_solution_ptr_map_.at(system::SolutionIndex(group, angle));
    auto& current_solution = group_solution_ptr_->GetSolution(angle);
    *stored_solution = current_solution;
  }
}

template class GroupSolveIteration<1>;
template class GroupSolveIteration<2>;
template class GroupSolveIteration<3>;

} // namespace bart::iteration::group