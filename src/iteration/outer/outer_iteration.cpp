#include "iteration/outer/outer_iteration.hpp"

#include "convergence/status.hpp"

namespace bart::iteration::outer {

template <typename ConvergenceType>
OuterIteration<ConvergenceType>::OuterIteration(std::unique_ptr<GroupIterator> group_iterator_ptr,
                                                std::unique_ptr<ConvergenceChecker> convergence_checker_ptr)
    : group_iterator_ptr_(std::move(group_iterator_ptr)),
      convergence_checker_ptr_(std::move(convergence_checker_ptr)) {
  AssertThrow(group_iterator_ptr_ != nullptr,
              dealii::ExcMessage("GroupSolveIteration pointer passed to OuterIteration constructor is null"))

  AssertThrow(convergence_checker_ptr_ != nullptr,
              dealii::ExcMessage("Convergence checker pointer passed to OuterIteration constructor is null"))
}

template <typename ConvergenceType>
void OuterIteration<ConvergenceType>::IterateToConvergence(system::System &system) {
  bool is_complete{ false };
  this->convergence_checker_ptr_->Reset();
  do {
    is_complete = Iterate(system);
    if (post_iteration_subroutine_ptr_ != nullptr) {
      data_names::StatusPort::Expose("===================== COMMENCING SUBROUTINE =====================\n");
      post_iteration_subroutine_ptr_->Execute(system);
      data_names::StatusPort::Expose("===================== COMPLETED SUBROUTINE  =====================\n");
    }
    ExposeIterationData(system);
  } while (!is_complete);
}

template <typename ConvergenceType>
void OuterIteration<ConvergenceType>::InnerIterationToConvergence(system::System &system) {
  group_iterator_ptr_->Iterate(system);
}

template<typename ConvergenceType>
auto OuterIteration<ConvergenceType>::Iterate(system::System &system) -> bool {
  for (int group = 0; group < system.total_groups; ++group) {
    for (int angle = 0; angle < system.total_angles; ++angle) {
      UpdateSystem(system, group, angle);
    }
  }

  InnerIterationToConvergence(system);

  auto convergence_status = CheckConvergence(system);
  if (convergence_status.delta.has_value()) {
    data_names::IterationErrorPort::Expose({convergence_status.iteration_number,
                                            convergence_status.delta.value()});
  }

  data_names::StatusPort::Expose("Outer iteration Status: ");
  data_names::ConvergenceStatusPort::Expose(convergence_status);

  return convergence_status.is_complete;
}

template<typename ConvergenceType>
auto OuterIteration<ConvergenceType>::ExposeIterationData(system::System &system) -> void {
  if (system.current_moments != nullptr) {
    data_names::SolutionMomentsPort::Expose(*system.current_moments);
    data_names::ScalarFluxPort::Expose(system.current_moments->GetMoment({0, 0, 0}));
  }

  if (system.right_hand_side_ptr_ != nullptr) {
    auto variable_terms = system.right_hand_side_ptr_->GetVariableTerms();

    if (variable_terms.contains(system::terms::VariableLinearTerms::kScatteringSource)) {
      auto scattering_source_ptr =
          system.right_hand_side_ptr_->GetVariableTermPtr(0, system::terms::VariableLinearTerms::kScatteringSource);
      if (scattering_source_ptr != nullptr) {
        dealii::Vector<double> scattering_source(*scattering_source_ptr);
        data_names::ScatteringSourcePort::Expose(scattering_source);
      }
    }
    if (variable_terms.contains(system::terms::VariableLinearTerms::kFissionSource)) {
      auto fission_source_ptr =
          system.right_hand_side_ptr_->GetVariableTermPtr(0, system::terms::VariableLinearTerms::kFissionSource);
      if (fission_source_ptr != nullptr) {
        dealii::Vector<double> fission_source(*fission_source_ptr);
        data_names::FissionSourcePort::Expose(fission_source);
      }
    }
  }
}

template class OuterIteration<double>;

} // namespace bart::iteration::outer