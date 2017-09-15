#ifndef __eigen_base_h__
#define __eigen_base_h__

#include "iteration_base.h"
#include "mg_base.h"

using namespace dealii;

template <int dim>
class EigenBase : public IterationBase<dim>
{
public:
  EigenBase (const ParameterHandler &prm);
  virtual ~EigenBase ();
  
  // override this in derived class for NDA-eigen
  virtual void do_iterations
  (std::vector<Vector<double> > &sflxes_proc,
   std::vector<std_cxx11::shared_ptr<EquationBase<dim> > > &equ_ptrs,
   std_cxx11::shared_ptr<IGBase<dim> > ig_ptr,
   std_cxx11::shared_ptr<MGBase<dim> > mg_ptr);
  
  virtual void eigen_iterations
  (std::vector<Vector<double> > &sflxes_proc,
   std::vector<std_cxx11::shared_ptr<EquationBase<dim> > > &equ_ptrs,
   std_cxx11::shared_ptr<IGBase<dim> > ig_ptr,
   std_cxx11::shared_ptr<MGBase<dim> > mg_ptr);
  
  virtual void update_prev_sflxes_fiss_src_keff
  (std::vector<Vector<double> >&sflxes_proc);
  
  void get_keff (double &k);

protected:
  void initialize_fiss_process
  (std::vector<Vector<double> > &sflxes_proc,
   std::vector<std_cxx11::shared_ptr<EquationBase<dim> > > &equ_ptrs);
  
  void calculate_fiss_src_keff
  (std::vector<Vector<double> > &sflxes_proc,
   std_cxx11::shared_ptr<EquationBase<dim> > equ_ptr);
  
  double estimate_fiss_src (std::vector<Vector<double> > &sflxes_proc);
  double estimate_k ();
  double estimate_k_diff ();

  const double err_k_tol;
  const double err_phi_tol;

  double keff;
  double keff_prev;
  double fiss_src;
  double fiss_src_prev;

  std::vector<Vector<double> > sflxes_proc_prev_eigen;
};

#endif //__eigen_base_h__