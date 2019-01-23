#ifndef BART_SRC_PROBLEM_PARAMETERS_DEALII_HANDLER_H_
#define BART_SRC_PROBLEM_PARAMETERS_DEALII_HANDLER_H_

#include <map>
#include <string>
#include <unordered_map>

#include <deal.II/base/parameter_handler.h>

#include "parameters_i.h"
#include "parameter_types.h"

namespace bart {

namespace problem {

/*!
 * \brief Problem parameters derived using a dealii ParameterHandler object.
 * 
 * The ParameterHandler can be given directly or a file that can be parsed into
 * a ParameterHandler object can be passed.
 */

class ParametersDealiiHandler : public ParametersI {
 public:
  struct KeyWords {
    /*!
     * \brief Data struct to contain keywords for input files
     * These are the strings that the ParameterHandler will look for in provided
     * input files.
     */
    const std::string kDiscretization_ = "ho spatial discretization";
    const std::string kEigenvalueProblem_ = "do eigenvalue calculations";
    const std::string kFEPolynomialDegree_ = "finite element polynomial degree";
    const std::string kFirstThermalGroup_ = "thermal group boundary";
    const std::string kHaveReflectiveBC_ = "have reflective boundary";
    const std::string kNCells_ = "number of cells for x, y, z directions";
    const std::string kNEnergyGroups_ = "number of groups";
    const std::string kOutputFilenameBase_ = "output file name base";
    const std::string kReflectiveBoundary_ = "reflective boundary names";
    const std::string kSpatialDimension_ = "problem dimension";
    const std::string kSpatialMax_ = "x, y, z max values of boundary locations";
    const std::string kTransportModel_ = "transport model";

    // Mesh parameters
    const std::string kMeshGenerated_ = "is mesh generated by deal.II";
    const std::string kMeshFilename_ = "mesh file name";
    const std::string kUniformRefinements_ = "uniform refinements";
    const std::string kFuelPinRadius_ = "fuel Pin radius";
    const std::string kFuelPinTriangulation_ = "triangulation type of fuel Pin";
    const std::string kMeshPinResolved_ = "is mesh pin-resolved";

    // Material parameters
    const std::string kMaterialSubsection_ = "material ID map";
    const std::string kMaterialMapFilename_ = "material id file name";
    const std::string kMaterialFilenames_ = "material id file name map";
    const std::string kNumberOfMaterials_ = "number of materials";
    const std::string kFuelPinMaterialMapFilename_ =
        "fuel pin material id file name";
    
    // Acceleration parameters
    const std::string kPreconditioner_ = "ho preconditioner name";
    const std::string kBSSOR_Factor_ = "ho ssor factor";
    const std::string kDoNDA_ = "do nda";
    const std::string kNDA_Discretization_ = "nda spatial discretization";
    const std::string kNDALinearSolver_ = "nda linear solver name";
    const std::string kNDAPreconditioner_ = "nda preconditioner name";
    const std::string kNDA_BSSOR_Factor_ = "nda ssor factor";
  
    // Solvers
    const std::string kEigenSolver_ = "eigen solver name";
    const std::string kInGroupSolver_ = "in group solver name";
    const std::string kLinearSolver_ = "ho linear solver name";
    const std::string kMultiGroupSolver_ = "mg solver name";

    // Angular quadrature
    const std::string kAngularQuad_ = "angular quadrature name";
    const std::string kAngularQuadOrder_ = "angular quadrature order";
  };
  
  ParametersDealiiHandler() = default;
  /*! Constructor that parses a given filename in the appropriate format to be
   * read by a ParameterHandler object.
   */
  ParametersDealiiHandler(const std::string filename);
  ~ParametersDealiiHandler() = default;

  /*! \brief Parses a ParameterHandler object for problem parameters */
  void Parse(dealii::ParameterHandler &handler);
  
  /*! \brief Set up a ParameterHandler object for problem parameters.
   * This setup includes default values.
   */
  void SetUp(dealii::ParameterHandler &handler);


  // Functions to get problem parameters
  // Basic Parameters ==========================================================
  DiscretizationType Discretization() const override { return discretization_; }

  bool IsEigenvalueProblem() const override { return is_eigenvalue_problem_; }

  int FEPolynomialDegree() const override { return fe_polynomial_degree_; }

  int FirstThermalGroup() const override { return first_thermal_group_; }

  bool HaveReflectiveBC() const override { return have_reflective_bc_; }

  EquationType TransportModel() const override { return transport_model_; }

  std::vector<int> NCells() const override { return n_cells_; }

  int NEnergyGroups() const override { return n_groups_; }

  std::string OutputFilenameBase() const override {
    return output_filename_base_; }

  std::map<Boundary, bool> ReflectiveBoundary() const override {
    return reflective_boundary_; }

  int SpatialDimension() const override { return spatial_dimension_; }

  std::vector<double> SpatialMax() const override { return spatial_max; }
  
  // MESH PARAMETERS ===========================================================
  int UniformRefinements() const override { return uniform_refinements_; }

  bool IsMeshGenerated() const override { return is_mesh_generated_; }

  std::string MeshFilename() const override { return mesh_file_name_; }

  double FuelPinRadius() const override { return fuel_pin_radius_; }

  FuelPinTriangulationType FuelPinTriangulation() const override {
    return fuel_pin_triangulation_; }

  bool IsMeshPinResolved() const override { return is_mesh_pin_resolved_;}

  // MATERIAL PARAMETERS =======================================================
  std::string MaterialMapFilename() const override { 
    return material_map_filename_; }

  std::unordered_map<int, std::string> MaterialFilenames() const override {
    return material_filenames_; }

  int NumberOfMaterials() const override { return n_materials_; }

  std::string FuelPinMaterialMapFilename() const override {
    return fuel_pin_material_map_filename_; }

  // Acceleration parameters ===================================================
  
  PreconditionerType Preconditioner() const override { return preconditioner_; }

  double BlockSSORFactor() const override { return block_ssor_factor_; }
  
  bool DoNDA() const override { return do_nda_; }
  
  LinearSolverType NDALinearSolver() const override {
    return nda_linear_solver_; }

  PreconditionerType NDAPreconditioner() const override {
    return nda_preconditioner_; }
  
  double NDABlockSSORFactor() const override {
    return nda_block_ssor_factor_;
  }

  DiscretizationType NDADiscretization() const override {
    return nda_discretization_; }
  
  // Solver Parameters =========================================================
  EigenSolverType EigenSolver() const override { return eigen_solver_; }

  InGroupSolverType InGroupSolver() const override { return in_group_solver_; }
  
  LinearSolverType LinearSolver() const override { return linear_solver_; }

  MultiGroupSolverType MultiGroupSolver() const override {
    return multi_group_solver_; }

  // Angular Quadrature Parameters =============================================
  AngularQuadType AngularQuad() const override { return angular_quad_; }

  int AngularQuadOrder() const override { return angular_quad_order_; }

  KeyWords GetKeyWords() const { return key_words_; }
  
 private:
  // Basic parameters
  DiscretizationType                   discretization_;
  bool                                 is_eigenvalue_problem_;
  int                                  fe_polynomial_degree_;
  int                                  first_thermal_group_;
  bool                                 have_reflective_bc_;
  EquationType                         transport_model_;
  std::vector<int>                     n_cells_;
  int                                  n_groups_;
  std::string                          output_filename_base_;
  std::map<Boundary, bool>             reflective_boundary_;    
  int                                  spatial_dimension_;
  std::vector<double>                  spatial_max;
                                       
  // Mesh parameters                   
  bool                                 is_mesh_generated_;
  std::string                          mesh_file_name_;
  int                                  uniform_refinements_;
  double                               fuel_pin_radius_;
  FuelPinTriangulationType             fuel_pin_triangulation_;
  bool                                 is_mesh_pin_resolved_;
                                       
  // Material Parameters
  std::string                          material_map_filename_;
  std::unordered_map<int, std::string> material_filenames_;
  int                                  n_materials_;
  std::string                          fuel_pin_material_map_filename_;
                                       
  // Acceleration parameters
  PreconditionerType                   preconditioner_;
  double                               block_ssor_factor_;
  bool                                 do_nda_;
  DiscretizationType                   nda_discretization_;
  LinearSolverType                     nda_linear_solver_;
  PreconditionerType                   nda_preconditioner_;
  double                               nda_block_ssor_factor_;
                                       
  // Solvers                           
  EigenSolverType                      eigen_solver_;
  InGroupSolverType                    in_group_solver_;
  LinearSolverType                     linear_solver_;
  MultiGroupSolverType                 multi_group_solver_;
                                       
  // Angular Quadrature                
  AngularQuadType                      angular_quad_;
  int                                  angular_quad_order_;
                                       
  // Key-words struct                  
  KeyWords                             key_words_;
  
  // Options mapping

  const std::unordered_map<std::string, Boundary> kBoundaryMap_ {
    {"xmin", Boundary::kXMin},
    {"xmax", Boundary::kXMax},
    {"ymin", Boundary::kYMin},
    {"ymax", Boundary::kYMax},
    {"zmin", Boundary::kZMin},
    {"zmax", Boundary::kZMax},
        }; /*!< Maps boundaries to strings used in parsed input files. */

  const std::unordered_map<std::string, DiscretizationType>
  kDiscretizationTypeMap_ {
    {"none", DiscretizationType::kNone},
    {"cfem",   DiscretizationType::kContinuousFEM},
    {"dfem", DiscretizationType::kDiscontinuousFEM},
        }; /*!< Maps discretization type to strings used in parsed input
            * files. */

  const std::unordered_map<std::string, EquationType> kEquationTypeMap_ {
    {"ep",   EquationType::kEvenParity},
    {"saaf", EquationType::kSelfAdjointAngularFlux},
    {"none", EquationType::kNone},
        }; /*!< Maps equation type to strings used in parsed input files. */

  const std::unordered_map<std::string, EigenSolverType> kEigenSolverTypeMap_ {
    {"pi",   EigenSolverType::kPowerIteration},
    {"none", EigenSolverType::kNone},
        }; /*!< Maps eigen solver type to strings used in parsed input files. */

  const std::unordered_map<std::string, FuelPinTriangulationType>
  kFuelPinTriangulationTypeMap_ {
    {"none",      FuelPinTriangulationType::kNone},
    {"simple",    FuelPinTriangulationType::kSimple},
    {"composite", FuelPinTriangulationType::kComposite},
        }; /*!< Maps fuel pin triangulation type to strings used in parsed input
            * files. */

  const std::unordered_map<std::string, InGroupSolverType>
  kInGroupSolverTypeMap_ {
    {"si",   InGroupSolverType::kSourceIteration},
    {"none", InGroupSolverType::kNone},
        }; /*!< Maps in-group solver type to strings used in parsed input
            * files. */
  
  const std::unordered_map<std::string, LinearSolverType> kLinearSolverTypeMap_ {
    {"cg",       LinearSolverType::kConjugateGradient},
    {"gmres",    LinearSolverType::kGMRES},
    {"bicgstab", LinearSolverType::kBiCGSTAB},
    {"direct",   LinearSolverType::kDirect},
    {"none",     LinearSolverType::kNone},
        };  /*!< Maps linear solver type to strings used in parsed input
             * files. */

  const std::unordered_map<std::string, MultiGroupSolverType>
  kMultiGroupSolverTypeMap_ {
    {"gs",   MultiGroupSolverType::kGaussSeidel},
    {"none", MultiGroupSolverType::kNone},
  }; /*!< Maps multi-group solver type to strings used in parsed input files. */

  const std::unordered_map<std::string, PreconditionerType>
  kPreconditionerTypeMap_ {
    {"amg",       PreconditionerType::kAMG},
    {"parasails", PreconditionerType::kParaSails},
    {"bjacobi",   PreconditionerType::kBlockJacobi},
    {"jacobi",    PreconditionerType::kJacobi},
    {"bssor",     PreconditionerType::kBlockSSOR},
    {"none",      PreconditionerType::kNone},
        }; /*!< Maps preconditioner type to strings used in parsed input
            * files. */

  const std::unordered_map<std::string, AngularQuadType> kAngularQuadTypeMap_ {
    {"lsgc", AngularQuadType::kLevelSymmetricGaussChebyshev},
    {"gl",   AngularQuadType::kGaussLegendre},
    {"none", AngularQuadType::kNone},
  }; /*!< Maps angular quadrature type to strings used in parsed input files. */

  // Setup functions
  /*! \brief Set up basic problem parameters */
  void SetUpBasicParameters(dealii::ParameterHandler &handler);
  /*! \brief Set up mesh parameters */
  void SetUpMeshParameters(dealii::ParameterHandler &handler);
  /*! \brief Set up material parameters */
  void SetUpMaterialParameters(dealii::ParameterHandler &handler);
  /*! \brief Set up acceleration parameters */
  void SetUpAccelerationParameters(dealii::ParameterHandler &handler);
  /*! \brief Set up solver parameters */
  void SetUpSolverParameters(dealii::ParameterHandler &handler);
  /*! \brief Set up angular quadrature parameters */
  void SetUpAngularQuadratureParameters(dealii::ParameterHandler &handler);
  
  
  /*! \brief Parses a ParameterHandler entry of type dealii::Patterns::List with
   * doubles into a vector.
   */
  std::vector<double> ParseDealiiList(std::string to_parse);
  std::vector<int>    ParseDealiiIntList(std::string to_parse);

  /*! \brief Parses the Material Filename mapping which is of the following
   * deal.II pattern: Map(Integer, Anything)
   */
  std::unordered_map<int, std::string> ParseMap(std::string to_parse);

  /*! \brief Parses a ParameterHandler entry of type
   * dealii::Patterns::MultipleSelection returning a map of one type to another
   */
  template<typename Key>
  std::map<Key, bool> ParseDealiiMultiple(
      const std::string to_parse,
      const std::unordered_map<std::string, Key> enum_map) const;

  /*! \brief Returns a string formed by combining the key strings in a mapping.
   * 
   * Entries will be separated by `|`. Used to generate valid option strings for
   * ParameterHandler entries. Optional parameter allows a list of options to
   * ignore.
   */
  template<typename T>
  std::string GetOptionString(
      const std::unordered_map<std::string, T> enum_map,
      const std::vector<T> to_ignore) const;
  template<typename T>
  std::string GetOptionString(
      const std::unordered_map<std::string, T> enum_map,
      const T to_ignore) const;
  template<typename T>
  std::string GetOptionString(
      const std::unordered_map<std::string, T> enum_map) const;

};

} // namespace problem

} // namespace bart

#endif // BART_SRC_PROBLEM_PARAMETERS_DEALII_HANDLER_H_
