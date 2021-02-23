#ifndef BART_SRC_SYSTEM_SYSTEM_HELPER_HPP_
#define BART_SRC_SYSTEM_SYSTEM_HELPER_HPP_

#include "system/system_helper_i.hpp"

namespace bart::system {

template <int dim>
class SystemHelper : public SystemHelperI<dim> {
 public:
  auto InitializeSystem(system::System& system_to_setup, const int total_groups, const int total_angles,
                        const bool is_eigenvalue_problem = true,
                        const bool is_rhs_boundary_term_variable = false) const -> void override;
  auto SetUpEnergyGroupToAngularSolutionPtrMap(solution::EnergyGroupToAngularSolutionPtrMap& to_setup,
                                               const int total_groups,
                                               const int total_angles) const -> void override;
  auto SetUpMPIAngularSolution(system::solution::MPIGroupAngularSolutionI &to_initialize,
                               const domain::DomainI<dim> &domain_definition,
                               const double value_to_set = 1.0) const -> void override;
  auto SetUpSystemTerms(system::System& system_to_setup,
                                const domain::DomainI<dim>& domain_definition) const -> void override;
  auto SetUpSystemMoments(system::System& system_to_setup,
                          const std::size_t solution_size) const -> void override;

};

} // namespace bart::system

#endif //BART_SRC_SYSTEM_SYSTEM_HELPER_HPP_
