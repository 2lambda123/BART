#ifndef __even_parity__
#define __even_parity__

#include "equation_base.h"

template<int dim>
class EvenParity : public EquationBase<dim>
{
public:
  EvenParity (std::string equation_name,
              const ParameterHandler &prm,
              const std_cxx11::shared_ptr<MeshGenerator<dim> > msh_ptr,
              const std_cxx11::shared_ptr<AQBase<dim> > aqd_ptr,
              const std_cxx11::shared_ptr<MaterialProperties> mat_ptr);
  ~EvenParity ();
  
  void pre_assemble_cell_matrices
  (typename DoFHandler<dim>::active_cell_iterator &cell,
   std::vector<std::vector<FullMatrix<double> > > &streaming_at_qp,
   std::vector<FullMatrix<double> > &collision_at_qp);
  
  void integrate_cell_bilinear_form
  (typename DoFHandler<dim>::active_cell_iterator &cell,
   FullMatrix<double> &cell_matrix,
   std::vector<std::vector<FullMatrix<double> > > &streaming_at_qp,
   std::vector<FullMatrix<double> > &collision_at_qp,
   const unsigned int &g,
   const unsigned int &i_dir);
  
  void integrate_boundary_bilinear_form
  (typename DoFHandler<dim>::active_cell_iterator &cell,
   unsigned int &fn,/*face number*/
   FullMatrix<double> &cell_matrix,
   const unsigned int &g,
   const unsigned int &i_dir);
  
  void integrate_interface_bilinear_form
  (typename DoFHandler<dim>::active_cell_iterator &cell,
   typename DoFHandler<dim>::cell_iterator &neigh,/*cell iterator for cell*/
   unsigned int &fn,/*concerning face number in local cell*/
   FullMatrix<double> &vi_ui,
   FullMatrix<double> &vi_ue,
   FullMatrix<double> &ve_ui,
   FullMatrix<double> &ve_ue,
   const unsigned int &g,
   const unsigned int &i_dir);
  
  void integrate_scattering_linear_form
  (typename DoFHandler<dim>::active_cell_iterator &cell,
   Vector<double> &cell_rhs,
   std::vector<Vector<double> > &sflx_proc,
   const unsigned int &g,
   const unsigned int &i_dir);
  
  void integrate_cell_fixed_linear_form
  (typename DoFHandler<dim>::active_cell_iterator &cell,
   Vector<double> &cell_rhs,
   std::vector<Vector<double> > &sflx_prev,
   const unsigned int &g,
   const unsigned int &i_dir);
  
private:
  double c_penalty;
  std::vector<double> tensor_norms;
};

#endif // __even_parity__
