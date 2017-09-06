#include <deal.II/base/index_set.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/grid/cell_id.h>

#include <deal.II/lac/petsc_solver.h>
#include <deal.II/lac/solver_bicgstab.h>

#include <algorithm>

#include "iterations.h"
#include "../aqdata/aq_base.h"
#include "../aqdata/aq_lsgc.h"

using namespace dealii;

template <int dim>
Iterations<dim>::Iterations
(const ParameterHandler &prm,
 const DoFHandler<dim> &dof_handler
 const std_cxx11::shared_ptr<MeshGenerator<dim> > msh_ptr,
 const std_cxx11::shared_ptr<AQBase<dim> > aqd_ptr,
 const std_cxx11::shared_ptr<MaterialProperties> mat_ptr)
:
transport_name (prm.get("transport model")),
is_eigen_problem(prm.get_bool("do eigenvalue calculations")),
do_nda(prm.get_bool("do NDA"))
{
  mat_ptr = build_material (prm);
  // vectors containing all the pointers to equations. Size is 2 if NDA is used.
  equ_ptrs.resize (do_nda?2:1);
  equ_ptrs[0] = build_equation (transport_name, prm, dof_handler, msh_ptr, aqd_ptr, mat_ptr);
  if (do_nda)
    equ_ptrs[1] = build_equation ("nda", prm, dof_handler, msh_ptr, aqd_ptr, mat_ptr);
}

template <int dim>
Iterations<dim>::~Iterations ()
{
}

template <int dim>
void Iterations<dim>::initialize_system_matrices_vectors
(SparsityPatternType &dsp, IndexSet &local_dofs)
{
  for (unsigned int i=0; i<equ_ptrs.size(); ++i)
    equ_ptrs[i]->initialize_system_matrices_vectors (dsp, local_dofs);
}

template <int dim>
void Iterations<dim>::solve_problems (std::vector<Vector<double> > &sflx_proc)
{
  if (is_eigen_problem)
  {
    std_cxx11::shared_ptr<EigenBase<dim> > pro_ptr = build_eigen_problem (prm);
    pro_ptr->do_iterations (local_cells, is_cell_at_bd, sflx_proc, equ_ptrs);
    pro_ptr->get_keff (keff);
  }
  else
  {
    std_cxx11::shared_ptr<MGBase<dim> > pro_ptr = build_mg_problem (prm);
    pro_ptr->do_iterations (local_cells, is_cell_at_bd, sflx_proc, equ_ptrs);
  }
}

template <int dim>
void Iterations<dim>

template <int dim>
void Iterations<dim>::get_keff (double &k)
{
  AssertThrow (is_eigen_problem,
               ExcMessage("Only eigen problems have keff"));
  k = this->keff;
}

// explicit instantiation to avoid linking error
template class Iterations<2>;
template class Iterations<3>;
