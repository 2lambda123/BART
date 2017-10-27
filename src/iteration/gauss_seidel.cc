#include "gauss_seidel.h"

template <int dim>
GaussSeidel<dim>::GaussSeidel (const ParameterHandler &prm)
:
MGBase<dim> (prm)
{
}

template <int dim>
GaussSeidel<dim>::~GaussSeidel ()
{
}

template <int dim>
void GaussSeidel<dim>::do_iterations
(std::vector<Vector<double> > &sflxes_proc,
 std::vector<std_cxx11::shared_ptr<EquationBase<dim> > > &equ_ptrs,
 std_cxx11::shared_ptr<IGBase<dim> > ig_ptr)
{
  AssertThrow (!this->is_eigen_problem,
               ExcMessage("This function shall not be called if it's eigen prob."));
  // purpose of overriding do_iteration is such that we provide another outer
  // iteration here if NDA is used
  if (this->do_nda)
  {
    AssertThrow (equ_ptrs.size()==2,
                 ExcMessage("There should be two equation pointers if do NDA"));
    // assemble bilinear forms of available equations
    for (unsigned int i=0; i<equ_ptrs.size(); ++i)
      equ_ptrs[i]->assemble_bilinear_form ();
    // do nothing for closure at the beginning
    equ_ptrs.back()->assemble_closure_bilinear_form (equ_ptrs.front(), false);
    // TODO: fill this up with NDA
  }
  else
  {
    AssertThrow (equ_ptrs.size()==1,
                 ExcMessage("There should be one equation pointer without NDA"));
    // assemble bilinear forms of available equations
    equ_ptrs.back()->assemble_bilinear_form ();
    // multigroup iterations. Note we would not need to override mg_iterations
    // until we want to do JFNK
    this->mg_iterations (sflxes_proc, equ_ptrs, ig_ptr);
  }
}

template <int dim>
void GaussSeidel<dim>::nonthermal_solves
(std::vector<Vector<double> > &sflxes_proc,
 std::vector<std_cxx11::shared_ptr<EquationBase<dim> > > &equ_ptrs,
 std_cxx11::shared_ptr<IGBase<dim> > ig_ptr)
{
  // loop over all nonthermal groups, assemble and solve one by one
  for (unsigned int g=0; g<this->g_thermal; ++g)
  {
    equ_ptrs.back()->assemble_linear_form (sflxes_proc, g);
    ig_ptr->solve_in_group (sflxes_proc, equ_ptrs.back(), g);
  }
}

template <int dim>
void GaussSeidel<dim>::thermal_iterations
(std::vector<Vector<double> > &sflxes_proc,
 std::vector<std_cxx11::shared_ptr<EquationBase<dim> > > &equ_ptrs,
 std_cxx11::shared_ptr<IGBase<dim> > ig_ptr)
{
  double err = 1.0;
  while (err>this->err_phi_tol)
  {
    double m = 0.0;
    for (unsigned int g=this->g_thermal; g<this->n_group; ++g)
    {
      // update scalar flux from previous mg iteration
      this->sflxes_proc_prev_mg[g] = sflxes_proc[g];
      // assemble rhs for current mg iteration at current group
      equ_ptrs.back()->assemble_linear_form (sflxes_proc, g);
      // solve for current group
      ig_ptr->solve_in_group (sflxes_proc, equ_ptrs.back(), g);
      // calculate the error up to this group
      m = std::max
      (m, this->estimate_phi_diff(sflxes_proc[g],
                                  this->sflxes_proc_prev_mg[g]));
    }
    err = m;
  }
}

template class GaussSeidel<2>;
template class GaussSeidel<3>;
