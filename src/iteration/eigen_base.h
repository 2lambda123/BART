#ifndef BART_SRC_ITERATION_EIGEN_BASE_H_
#define BART_SRC_ITERATION_EIGEN_BASE_H_

#include <memory>

#include "iteration_base.h"
#include "mg_base.h"

//! This class provides eigenvalue calculations foundations.
/*!
 This class is the base class of eigenvalue calculations. It will operate the
 iteration at a high level without touching specifics in the equations. Alternatively,
 it interfaces with MGBase objects iteratively.

 \author Weixiong Zheng
 \date 2017/09
 */
template <int dim>
class EigenBase : public IterationBase<dim> {
 public:
  /*!
    Enumerator for the types of eigenvalue iteration
   */
  enum class EigenIterationType {
    kPowerIteration /*!< Power iteration */
  };
  /*!
   * \brief Static factory for classes derived from EigenBase
   *
   * Instantiates and returns the appropriate eigenvalue iteration class based
   * on the value specified in the problem as `eigen solver name`.
   *
   * \param prm ParameterHandler object containing problem definition.
   * 
   */j
  static std::unique_ptr<EigenBase<dim>> CreateEigenIteration (
      const dealii::ParameterHandler &prm,
      std::shared_ptr<FundamentalData<dim>> &dat_ptr);
  
  EigenBase (const dealii::ParameterHandler &prm,
      std::shared_ptr<FundamentalData<dim>> &dat_ptr);
  virtual ~EigenBase () = default;

  /*!
   This function calls eigen_iterations to performs eigenvalue iterations.

   \todo Add NDA functionality.
   */
  virtual void DoIterations (std::unordered_map<std::string,
      std::unique_ptr<EquationBase<dim>>> &equ_ptrs);

  /*!
   This virtual function performs eigenvalue iteration after overriding with the
   std::vector<std_cxx11::shared_ptr<EquationBase<dim> > >::back().

   \return Void.
   */
  virtual void EigenIterations
  (std::unique_ptr<EquationBase<dim>> &equ_ptrs) = 0;

  /*!
   Function to update \f$\phi\f$, fission source and \f$k_\mathrm{eff}\f$ from
   previous iteration with current values.

   \param equ_name Name of the equation used for updating scalar fluxes.
   \return Void
   */
  virtual void UpdatePrevSflxesFissSrcKeff (const std::string & equ_name);

  /*!
   Function to get \f$k_\mathrm{eff}\f$ value from eigenvalue iterations.

   \return \f$k_\mathrm{eff}\f$ value (double type).
   */
  double GetKeff () const;

 protected:
  /*!
   Function to estimate fission source and \f$k_\mathrm{eff}\f$ values.

   \param sflxes_proc
   \return Void.
   */
  void EstimateFissSrcKeff (std::unique_ptr<EquationBase<dim>> &equ_ptr);

  /*!
   Function to calculate \f$k_\mathrm{eff}\f$.

   \return \f$k_\mathrm{eff}\f$ value.
   */
  double EstimateKeff ();

  /*!
   Function to calculate difference between \f$k_\mathrm{eff}\f$ and
   \f$k_\mathrm{eff,\ prev}\f$.

   \return The relative difference (double type).
   */
  double EstimateKDiff ();

  std::unique_ptr<MGBase<dim>> mg_ptr_;

  const double err_k_tol_;//!< Tolerance for convergence check on \f$k_\mathrm{eff}\f$
  const double err_phi_tol_;//!< Tolerance for convergence check on \f$\phi\f$

  double keff_;//!< \f$k_\mathrm{eff}\f$ value.
  double keff_prev_;//!< \f$k_\mathrm{eff}\f$ value from previous eigen iteration.
  double fiss_src_;//!< Fission source.
  double fiss_src_prev_;//!< Fission source from previous eigen iteration.
};

#endif //BART_SRC_ITERATION_EIGEN_BASE_H_
