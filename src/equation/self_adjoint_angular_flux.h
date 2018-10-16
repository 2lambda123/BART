#ifndef BART_SRC_EQUATION_SELF_ADJOINT_ANGULAR_FLUX_H_
#define BART_SRC_EQUATION_SELF_ADJOINT_ANGULAR_FLUX_H_

#include "equation_base.h"

/*!
 * \brief Provides the weak formulation for the Self Adjoint Angular Flux 
 *         (SAAF) equation.
 *
 * This is a second-order formulation of the transport equation, given as:
 * \f[
 * -\vec{\Omega} \cdot \nabla \frac{1}{\Sigma_t}\vec{\Omega}\cdot\nabla\psi
 * + \Sigma_t\psi = \frac{1}{4\pi}\left[\Sigma_s\phi + Q - \vec{\Omega}\cdot
 * \nabla\frac{\Sigma_s\phi + Q}{\Sigma_t}\right]
 * \f]
 * 
 * \author Joshua Rehak
 */

template <int dim>
class SelfAdjointAngularFlux : public EquationBase<dim> {
 public:
  /*! Class constructor */
  SelfAdjointAngularFlux(const std::string equation_name,
                         const dealii::ParameterHandler &prm,
                         std::shared_ptr<FundamentalData<dim>> &data_ptr);
  ~SelfAdjointAngularFlux() = default;

  /*!
   * \brief Provides the integrated value of the linear scattering term.
   */
  void IntegrateScatteringLinearForm (
      typename dealii::DoFHandler<dim>::active_cell_iterator &cell,
      dealii::Vector<double> &cell_rhs,
      const int &g,
      const int &dir) override;  
  void PreassembleCellMatrices () override;

  // NOT DONE YET
  
  
  void IntegrateCellBilinearForm (
      typename dealii::DoFHandler<dim>::active_cell_iterator &cell,
      dealii::FullMatrix<double> &cell_matrix,
      const int &g,
      const int &dir) override {};
  void IntegrateBoundaryBilinearForm (
      typename dealii::DoFHandler<dim>::active_cell_iterator &cell,
      const int &fn,
      dealii::FullMatrix<double> &cell_matrix,
      const int &g,
      const int &dir) override {};

  void IntegrateInterfaceBilinearForm (
      typename dealii::DoFHandler<dim>::active_cell_iterator &cell,
      typename dealii::DoFHandler<dim>::cell_iterator &neigh,/*cell iterator for cell*/
      const int &fn,/*concerning face number in local cell*/
      dealii::FullMatrix<double> &vi_ui,
      dealii::FullMatrix<double> &vi_ue,
      dealii::FullMatrix<double> &ve_ui,
      dealii::FullMatrix<double> &ve_ue,
      const int &g,
      const int &i_dir) override {};
  void IntegrateBoundaryLinearForm (
      typename dealii::DoFHandler<dim>::active_cell_iterator &cell,
      const int &fn,/*face number*/
      dealii::Vector<double> &cell_rhs,
      const int &g,
      const int &dir) override {};  
  void IntegrateCellFixedLinearForm (
      typename dealii::DoFHandler<dim>::active_cell_iterator &cell,
      dealii::Vector<double> &cell_rhs,
      const int &g,
      const int &dir) override {};
 protected:
  using EquationBase<dim>::xsec_;
  using EquationBase<dim>::pre_streaming_;
  using EquationBase<dim>::pre_collision_;
  using EquationBase<dim>::omega_;
  using EquationBase<dim>::equ_name_;
  using EquationBase<dim>::fv_;
  using EquationBase<dim>::dat_ptr_;
  using EquationBase<dim>::mat_vec_;
  using EquationBase<dim>::n_group_;
  using EquationBase<dim>::n_q_;
  using EquationBase<dim>::n_dir_;
  using EquationBase<dim>::dofs_per_cell_;
  
  dealii::FullMatrix<double> CellCollisionMatrix (int q);
  dealii::FullMatrix<double> CellStreamingMatrix (int q, int dir);
};

#endif // BART_SRC_EQUATION_SELF_ADJOINT_ANGULAR_FLUX_H_
