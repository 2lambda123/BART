#ifndef BART_SRC_SYSTEM_SYSTEM_FUNCTIONS_H_
#define BART_SRC_SYSTEM_SYSTEM_FUNCTIONS_H_

#include "system/solution/mpi_group_angular_solution_i.h"
#include "domain/definition_i.h"
#include "system/system.h"
#include "system/solution/solution_types.h"

namespace bart {

namespace system {

/*! \brief Initializes all solutions and sets to a given value */
template <int dim>
void SetUpMPIAngularSolution(
    system::solution::MPIGroupAngularSolutionI &to_initialize,
    const domain::DefinitionI<dim> &domain_definition,
    const double value_to_set = 1.0);

/*! \brief Initializes system, instantiates RHS/LHS and moments classes.
 *
 * This function initializes a system by setting total groups, total angles,
 * initial keffective (set to 1.0) and instantiating the RHS, LHS, and moments
 * classes.
 *
 * @param system_to_setup system to initialize
 * @param total_groups total number of energy group
 * @param total_angles total number of angles
 * @param is_eigenvalue_problem identifies if problem is an eigenvalue problem
 */
void InitializeSystem(system::System& system_to_setup,
                      const int total_groups,
                      const int total_angles,
                      const bool is_eigenvalue_problem = true);

template <int dim>
void SetUpSystemTerms(system::System& system_to_setup,
                      const domain::DefinitionI<dim>& domain_definition);

void SetUpSystemMoments(system::System& system_to_setup,
                        const std::size_t solution_size);

void SetUpEnergyGroupToAngularSolutionPtrMap(
    solution::EnergyGroupToAngularSolutionPtrMap& to_setup,
    const int total_groups,
    const int total_angles);

} // namespace system

} // namespace bart

#endif //BART_SRC_SYSTEM_SYSTEM_FUNCTIONS_H_
