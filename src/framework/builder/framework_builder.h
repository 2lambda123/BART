#ifndef BART_SRC_FRAMEWORK_BUILDER_FRAMEWORK_BUILDER_H_
#define BART_SRC_FRAMEWORK_BUILDER_FRAMEWORK_BUILDER_H_

#include <memory>

// Problem parameters
#include "problem/parameters_i.h"

// Interface classes built by this factory

#include "convergence/reporter/mpi_i.h"
#include "convergence/final_i.h"
#include "domain/definition_i.h"
#include "domain/finite_element/finite_element_i.h"
#include "quadrature/quadrature_set_i.h"
#include "solver/group/single_group_solver_i.h"


namespace bart {

namespace framework {

namespace builder {

template <int dim>
class FrameworkBuilder {
 public:
  FrameworkBuilder() = default;
  ~FrameworkBuilder() = default;

  using ParametersType = const problem::ParametersI&;

  using DomainType = domain::DefinitionI<dim>;
  using FiniteElementType = domain::finite_element::FiniteElementI<dim>;
  using MomentConvergenceCheckerType = convergence::FinalI<system::moments::MomentVector>;
  using ParameterConvergenceCheckerType = convergence::FinalI<double>;
  using QuadratureSetType = quadrature::QuadratureSetI<dim>;
  using ReporterType = convergence::reporter::MpiI;
  using SingleGroupSolverType = solver::group::SingleGroupSolverI;

  std::unique_ptr<ReporterType> BuildConvergenceReporter();
  std::unique_ptr<DomainType> BuildDomain(ParametersType,
                                          const std::shared_ptr<FiniteElementType>&,
                                          std::string material_mapping);
  std::unique_ptr<FiniteElementType> BuildFiniteElement(ParametersType);
  std::unique_ptr<MomentConvergenceCheckerType> BuildMomentConvergenceChecker(
      double max_delta, int max_iterations);
  std::unique_ptr<ParameterConvergenceCheckerType> BuildParameterConvergenceChecker(
      double max_delta, int max_iterations);
  std::shared_ptr<QuadratureSetType> BuildQuadratureSet(ParametersType);
  std::unique_ptr<SingleGroupSolverType> BuildSingleGroupSolver(
      const int max_iterations = 1000,
      const double convergence_tolerance = 1e-10);

};

} // namespace builder

} // namespace framework

} // namespace bart

#endif //BART_SRC_FRAMEWORK_BUILDER_FRAMEWORK_BUILDER_H_
