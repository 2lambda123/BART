#include "framework/framework_helper.hpp"

#include "framework/framework.hpp"
#include "framework/builder/framework_validator.hpp"
#include "instrumentation/builder/instrument_builder.hpp"
#include "material/material_protobuf.h"
#include "results/output_dealii_vtu.h"
#include "system/system_helper.hpp"
#include "system/solution/solution_types.h"

#include <fstream>

#include <fmt/color.h>
#include <system/system_helper.hpp>

namespace bart::framework {

namespace  {

template <typename T> inline std::shared_ptr<T> Shared(std::unique_ptr<T> to_convert_ptr) { return to_convert_ptr; }
} // namespace

template<int dim>
FrameworkHelper<dim>::FrameworkHelper(const std::shared_ptr<SystemHelper>& system_helper_ptr)
    : system_helper_ptr_(system_helper_ptr) {
  AssertThrow(system_helper_ptr_ != nullptr, dealii::ExcMessage("Error in constructor of framework helper, system"
                                                                "helper pointer passed is null"))
}

template<int dim>
auto FrameworkHelper<dim>::ToFrameworkParameters(
    const problem::ParametersI &problem_parameters) -> framework::FrameworkParameters {
  using Boundary = problem::Boundary;

  framework::FrameworkParameters return_parameters {
    .output_filename_base{ problem_parameters.OutputFilenameBase()},
    .neutron_energy_groups{ problem_parameters.NEnergyGroups() },
    .equation_type{ problem_parameters.TransportModel() },
    .group_solver_type{ problem_parameters.InGroupSolver() },
    .angular_quadrature_type{ problem_parameters.AngularQuad() },
    .angular_quadrature_order{ quadrature::Order(problem_parameters.AngularQuadOrder()) },
    .spatial_dimension{ framework::FrameworkParameters::SpatialDimension(problem_parameters.SpatialDimension()) },
    .domain_size{ framework::FrameworkParameters::DomainSize(problem_parameters.SpatialMax()) },
    .number_of_cells{ framework::FrameworkParameters::NumberOfCells(problem_parameters.NCells()) },
    .uniform_refinements{ problem_parameters.UniformRefinements() },
    .discretization_type{ problem_parameters.Discretization() },
    .polynomial_degree{ framework::FrameworkParameters::PolynomialDegree(problem_parameters.FEPolynomialDegree()) }
  };

  std::set<Boundary> reflective_boundaries;
  for (const auto& [boundary, is_reflective] : problem_parameters.ReflectiveBoundary()){
    if (is_reflective)
      reflective_boundaries.insert(boundary);
  }
  return_parameters.reflective_boundaries = reflective_boundaries;

  // Open material mapping file and read
  std::ifstream mapping_file(problem_parameters.MaterialMapFilename());
  if (mapping_file.is_open()) {
    return_parameters.material_mapping = std::string((std::istreambuf_iterator<char>(mapping_file)),
                                                     std::istreambuf_iterator<char>());
  } else {
    AssertThrow(false, dealii::ExcMessage("Failed to open material mapping file."))
  }

  const auto eigen_solver_type{ problem_parameters.EigenSolver() };
  const bool is_eigenvalue_solve{ eigen_solver_type != problem::EigenSolverType::kNone };

  if (is_eigenvalue_solve) {
    return_parameters.eigen_solver_type = eigen_solver_type;
  }

  MaterialProtobuf materials(problem_parameters.MaterialFilenames(),
                             is_eigenvalue_solve,
                             false,
                             return_parameters.neutron_energy_groups,
                             problem_parameters.NumberOfMaterials());

  return_parameters.cross_sections_ = std::make_shared<data::CrossSections>(materials);

  return return_parameters;
}

template<int dim>
auto FrameworkHelper<dim>::BuildFramework(
    builder::FrameworkBuilderI<dim>& builder,
    const framework::FrameworkParameters& parameters) -> std::unique_ptr<framework::FrameworkI> {
  using FrameworkPart = framework::builder::FrameworkPart;
  using MomentCalculator = typename builder::FrameworkBuilderI<dim>::MomentCalculator;
  using OuterIteration = typename builder::FrameworkBuilderI<dim>::OuterIteration;
  using MomentCalculatorImpl = typename builder::FrameworkBuilderI<dim>::MomentCalculatorImpl;
  using UpdaterPointers = typename builder::FrameworkBuilderI<dim>::UpdaterPointers;
  using QuadratureSet = typename builder::FrameworkBuilderI<dim>::QuadratureSet;
  using Validator = framework::builder::FrameworkValidator;

//  using InstrumentBuilder = instrumentation::builder::InstrumentBuilder;
//  using InstrumentName = instrumentation::builder::InstrumentName;
//  using ColorStringPair = std::pair<utility::Color, std::string>;
//
//  // Build instruments to be used
//  auto status_instrument = Shared(
//      InstrumentBuilder::BuildInstrument<ColorStringPair>(InstrumentName::kColorStatusToConditionalOstream));
//  auto convergence_status_instrument_ptr = Shared(
//      InstrumentBuilder::BuildInstrument<convergence::Status>(InstrumentName::kConvergenceStatusToConditionalOstream));
//  auto string_instrument_ptr = Shared(
//      InstrumentBuilder::BuildInstrument<std::string>(InstrumentName::kStringToConditionalOstream));

  Validator validator;
//  using ValidatorStatusPort = framework::builder::data_port::ValidatorStatusPort;
//  instrumentation::GetPort<ValidatorStatusPort>(validator).AddInstrument(status_instrument);
  validator.Parse(parameters);

  const int n_groups{ parameters.neutron_energy_groups };
  const bool need_angular_solution_storage{ validator.NeededParts().contains(FrameworkPart::AngularSolutionStorage) };
  const bool has_reflective_boundaries { !parameters.reflective_boundaries.empty() };
  const std::string output_filename_base { parameters.output_filename_base };

  fmt::print(fg(fmt::color::green) | fmt::emphasis::bold, "Building framework: {}\n", parameters.name);

  auto finite_element_ptr = Shared(builder.BuildFiniteElement(parameters.cell_finite_element_type,
                                                              parameters.discretization_type,
                                                              parameters.polynomial_degree));
  auto domain_ptr = Shared(builder.BuildDomain(parameters.domain_size, parameters.number_of_cells,
                                               finite_element_ptr, parameters.material_mapping));

  fmt::print("Setting up domain...\n");
  domain_ptr->SetUpMesh(parameters.uniform_refinements);
  domain_ptr->SetUpDOF();

  // These objects will be set up differently depending on the implementation
  std::shared_ptr<QuadratureSet> quadrature_set_ptr{ nullptr };
  UpdaterPointers updater_pointers;
  std::unique_ptr<MomentCalculator> moment_calculator_ptr{ nullptr };


  // Set up for Angular/Scalar solve
  if (parameters.equation_type != problem::EquationType::kDiffusion) {
    // Angular solve
    AssertThrow(parameters.angular_quadrature_order.has_value(),
                dealii::ExcMessage("Error building framework, equation type requires quadrature but order is null"))
    quadrature_set_ptr = builder.BuildQuadratureSet(parameters.angular_quadrature_type,
                                                    parameters.angular_quadrature_order.value());
    moment_calculator_ptr = builder.BuildMomentCalculator(quadrature_set_ptr, MomentCalculatorImpl::kZerothMomentOnly);
  } else {
    // Scalar solve
    moment_calculator_ptr = builder.BuildMomentCalculator(MomentCalculatorImpl::kScalarMoment);
  }

  const int n_angles { quadrature_set_ptr == nullptr ? 1 : static_cast<int>(quadrature_set_ptr->size()) };

  // Set up angular solutions if needed
  system::solution::EnergyGroupToAngularSolutionPtrMap angular_solutions_;
  if (need_angular_solution_storage)
    system_helper_ptr_->SetUpEnergyGroupToAngularSolutionPtrMap(angular_solutions_, n_groups, n_angles);

  //TODO: Add overload that makes this unecessary
  std::map<problem::Boundary, bool> reflective_boundaries {
      {problem::Boundary::kXMin, false}, {problem::Boundary::kXMax, false},
      {problem::Boundary::kYMin, false}, {problem::Boundary::kYMax, false},
      {problem::Boundary::kZMin, false}, {problem::Boundary::kZMax, false},
  };
  for (auto& boundary : parameters.reflective_boundaries) {
    reflective_boundaries.at(boundary) = true;
  }

  // Formulation specific builds
  if (parameters.equation_type == problem::EquationType::kSelfAdjointAngularFlux) {
    auto saaf_formulation_ptr = builder.BuildSAAFFormulation(finite_element_ptr,
                                                             parameters.cross_sections_.value(),
                                                             quadrature_set_ptr,
                                                             formulation::SAAFFormulationImpl::kDefault);
    saaf_formulation_ptr->Initialize(domain_ptr->Cells().at(0));
    if (has_reflective_boundaries) {
      updater_pointers = builder.BuildUpdaterPointers(std::move(saaf_formulation_ptr),
                                                      builder.BuildStamper(domain_ptr),
                                                      quadrature_set_ptr,
                                                      reflective_boundaries,
                                                      angular_solutions_);
    } else {
      updater_pointers = builder.BuildUpdaterPointers(std::move(saaf_formulation_ptr),
                                                      builder.BuildStamper(domain_ptr),
                                                      quadrature_set_ptr);
    }
  } else if (parameters.equation_type == problem::EquationType::kDiffusion) {
    auto diffusion_formulation_ptr = builder.BuildDiffusionFormulation(finite_element_ptr,
                                                                       parameters.cross_sections_.value(),
                                                                       formulation::DiffusionFormulationImpl::kDefault);
    diffusion_formulation_ptr->Precalculate(domain_ptr->Cells().at(0));
    updater_pointers = builder.BuildUpdaterPointers(std::move(diffusion_formulation_ptr),
                                                    builder.BuildStamper(domain_ptr),
                                                    reflective_boundaries);
  }

  auto initializer_ptr = builder.BuildInitializer(updater_pointers.fixed_updater_ptr,
                                                  parameters.neutron_energy_groups,
                                                  n_angles);

  auto group_solution_ptr = Shared(builder.BuildGroupSolution(n_angles));
  system_helper_ptr_->SetUpMPIAngularSolution(*group_solution_ptr, *domain_ptr, 1.0);

  auto group_iteration_ptr = builder.BuildGroupSolveIteration(
      builder.BuildSingleGroupSolver(10000, 1e-10),
      builder.BuildMomentConvergenceChecker(1e-6, 10000),
      std::move(moment_calculator_ptr),
      group_solution_ptr,
      updater_pointers,
      builder.BuildMomentMapConvergenceChecker(1e-6, 1000));

  if (need_angular_solution_storage) {
    group_iteration_ptr->UpdateThisAngularSolutionMap(angular_solutions_);
    validator.AddPart(FrameworkPart::AngularSolutionStorage);
  }

  std::unique_ptr<OuterIteration> outer_iteration_ptr{ nullptr };

  if (parameters.eigen_solver_type.has_value()){
    outer_iteration_ptr = builder.BuildOuterIteration(std::move(group_iteration_ptr),
                                                      builder.BuildParameterConvergenceChecker(1e-6, 1000),
                                                      builder.BuildKEffectiveUpdater(finite_element_ptr,
                                                                                     parameters.cross_sections_.value(),
                                                                                     domain_ptr),
                                                      updater_pointers.fission_source_updater_ptr);
  } else {
    outer_iteration_ptr = builder.BuildOuterIteration(std::move(group_iteration_ptr),
                                                      builder.BuildParameterConvergenceChecker(1e-6, 1000));
  }

  auto system_ptr = builder.BuildSystem(parameters.neutron_energy_groups,
                                        n_angles,
                                        *domain_ptr,
                                        group_solution_ptr->GetSolution(0).size(),
                                        parameters.eigen_solver_type.has_value(),
                                        need_angular_solution_storage);

  validator.ReportValidation();

  return std::make_unique<framework::Framework>(std::move(system_ptr),
                                                std::move(initializer_ptr),
                                                std::move(outer_iteration_ptr),
                                                std::make_unique<results::OutputDealiiVtu<dim>>(domain_ptr));
}

template class FrameworkHelper<1>;
template class FrameworkHelper<2>;
template class FrameworkHelper<3>;

} // namespace bart::framework
