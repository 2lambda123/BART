#include <deal.II/fe/fe_values.h>

#include <boost/algorithm/string.hpp>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/grid/cell_id.h>

#include <deal.II/lac/petsc_solver.h>
#include <deal.II/lac/solver_bicgstab.h>

#include <algorithm>
#include <fstream>
#include <sstream>

#include "bart_driver.h"
#include "../aqdata/lsgc.h"
#include "../equation/even_parity.h"
#include "../iteration/power_iteration.h"
#include "../iteration/gauss_sidel.h"


template <int dim>
BartDriver<dim>::BartDriver (ParameterHandler &prm)
:
triangulation (MPI_COMM_WORLD,
               typename Triangulation<dim>::MeshSmoothing
               (Triangulation<dim>::smoothing_on_refinement |
                Triangulation<dim>::smoothing_on_coarsening)),
dof_handler (triangulation),
pcout(std::cout,
      (Utilities::MPI::this_mpi_process(MPI_COMM_WORLD) == 0)),
transport_model_name(prm.get("transport model")),
aq_name(prm.get("angular quadrature name")),
n_group(prm.get_integer("number of groups")),
n_azi(prm.get_integer("angular quadrature order")),
is_eigen_problem(prm.get_bool("do eigenvalue calculations")),
do_nda(prm.get_bool("do NDA")),
have_reflective_bc(prm.get_bool("have reflective BC")),
p_order(prm.get_integer("finite element polynomial degree")),
global_refinements(prm.get_integer("uniform refinements")),
namebase(prm.get("output file name base")),
ho_linear_solver_name(prm.get("HO linear solver name")),
ho_preconditioner_name(prm.get("HO preconditioner name"))
{
  sflxes_proc.resize (n_group);
  build_basis (prm);
}

template <int dim>
BartDriver<dim>::~BartDriver ()
{
  dof_handler.clear();
}

template <int dim>
void BartDriver<dim>::build_basis (ParameterHandler &prm)
{
  // TODO: Seperate builders. Testing shows linker error when seperating. So this
  // is left for future improvement
  
  // build pointer to Iterations
  itr_ptr = std_cxx11::shared_ptr<Iterations<dim> > (new Iterations<dim>(prm));
  
  // build pointer to angular quadrature data
  build_aq_model (aqd_ptr, prm);
  
  // get angular infomation
  n_total_ho_vars = aqd_ptr->get_n_total_ho_vars ();
  n_azi = aqd_ptr->get_sn_order ();
  n_dir = aqd_ptr->get_n_dir ();
  
  // build pointers to mesh constructors and data
  msh_ptr = std_cxx11::shared_ptr<MeshGenerator<dim> >(new MeshGenerator<dim>(prm));
  
  // build pointers to material data
  mat_ptr = std_cxx11::shared_ptr<MaterialProperties>(new MaterialProperties(prm));
  
  // initialize finite element space
  if (discretization=="dfem")
    fe = new FE_DGQ<dim> (p_order);
  else
    fe = new FE_Q<dim> (p_order);
  
  // instantiate equation pointers
  equ_ptrs.resize (do_nda?2:1);
  build_equation
  (equ_ptrs.front(), transport_model_name, prm, msh_ptr, aqd_ptr, mat_ptr);
  if (do_nda)
    build_equation (equ_ptrs.back(), "nda", prm, msh_ptr, aqd_ptr, mat_ptr);
  
  // instantiate in group iterations
  build_ig_iterations (ig_ptr, prm);
  // instantiate multigroup iterations
  build_mg_iterations (mg_ptr, prm);
  // instantiate eigen iterations if this is eigen problem
  if (is_eigen_problem)
    build_eigen_iterations (eig_ptr, prm);
}

template <int dim>
void BartDriver<dim>::report_system ()
{
  pcout << "SN quadrature order: " << n_azi << std::endl
  << "Number of angles: " << n_dir << std::endl
  << "Number of groups: " << n_group << std::endl;

  pcout << "Transport model: " << transport_model_name << std::endl;
  pcout << "Spatial discretization: " << discretization << std::endl;
  pcout << "HO linear solver: " << ho_linear_solver_name << std::endl;
  if (ho_linear_solver_name!="direct")
    pcout << "HO preconditioner: " << ho_preconditioner_name << std::endl;
  pcout << "do NDA? " << do_nda << std::endl;
  
  pcout << "Number of cells: " << triangulation.n_global_active_cells() << std::endl;
  pcout << "High-order total DoF counts: " << n_total_ho_vars*dof_handler.n_dofs() << std::endl;

  if (is_eigen_problem)
    pcout << "Problem type: k-eigenvalue problem" << std::endl;
  if (do_nda)
    pcout << "NDA total DoF counts: " << n_group*dof_handler.n_dofs() << std::endl;
  pcout << "is eigenvalue problem? " << is_eigen_problem << std::endl;
}

template <int dim>
void BartDriver<dim>::build_equation
(std_cxx11::shared_ptr<EquationBase<dim> > equ_ptr,
 std::string equation_name,
 const ParameterHandler &prm,
 const std_cxx11::shared_ptr<MeshGenerator<dim> > msh_ptr,
 const std_cxx11::shared_ptr<AQBase<dim> > aqd_ptr,
 const std_cxx11::shared_ptr<MaterialProperties> mat_ptr)
{
  // TODO: add NDA to it after having NDA class
  std_cxx11::shared_ptr<EquationBase<dim> > equation_pointer;
  if (equation_name=="ep")
    equ_ptr =
    std_cxx11::shared_ptr<EquationBase<dim> >
    (new EvenParity<dim> (equation_name, prm, msh_ptr, aqd_ptr, mat_ptr));
}

template <int dim>
void BartDriver<dim>::build_aq_model
(std_cxx11::shared_ptr<AQBase<dim> > aqd_ptr, ParameterHandler &prm)
{
  std::string aq_name = prm.get ("angular quadrature name");
  AssertThrow (aq_name!="none",
               ExcMessage("angular quadrature name incorrect or missing"));
  // TODO: add more angular quadratures
  if (aq_name=="lsgc")
    aqd_ptr = std_cxx11::shared_ptr<AQBase<dim> > (new LSGC<dim>(prm));
  //return aq_pointer;
}

/** \brief Function used to build pointer to instance of InGroupBase's derived class
 */
template <int dim>
void BartDriver<dim>::build_eigen_iterations
(std_cxx11::shared_ptr<EigenBase<dim> > eig_ptr, const ParameterHandler &prm)
{
  // TODO: we only have power iteration now, change later once we need to choose
  // different in group solvers
  eig_ptr =
  std_cxx11::shared_ptr<EigenBase<dim> >
  (new PowerIteration<dim> (prm));
}

/** \brief Function used to build pointer to instance of MGBase's derived class
 */
template <int dim>
void BartDriver<dim>::build_mg_iterations
(std_cxx11::shared_ptr<MGBase<dim> > mg_ptr, const ParameterHandler &prm)
{
  // TODO: fill this up once we have derived class of MGBase
  mg_ptr =
  std_cxx11::shared_ptr<MGBase<dim> > (new GaussSidel<dim> (prm));
}

/** \brief Function used to build pointer to instance of InGroupBase's derived class
 */
template <int dim>
void BartDriver<dim>::build_ig_iterations
(std_cxx11::shared_ptr<IGBase<dim> > ig_ptr, const ParameterHandler &prm)
{
  // TODO: we only have source iteration now, change later once we need to choose
  // different in group solvers
  bool do_nda = prm.get_bool ("do NDA");
  if (!do_nda)
    ig_ptr =
    std_cxx11::shared_ptr<IGBase<dim> > (new SourceIteration<dim> (prm));
}

template <int dim>
void BartDriver<dim>::setup_system ()
{
  pcout << "setup system" << std::endl;
  dof_handler.distribute_dofs (*fe);
  
  local_dofs = dof_handler.locally_owned_dofs ();
  DoFTools::extract_locally_relevant_dofs (dof_handler,
                                           relevant_dofs);
  
  constraints.clear ();
  constraints.reinit (relevant_dofs);
  DoFTools::make_hanging_node_constraints (dof_handler,
                                           constraints);
  constraints.close ();
  
  DynamicSparsityPattern dsp (relevant_dofs);
  
  if (discretization=="dfem")
    DoFTools::make_flux_sparsity_pattern (dof_handler,
                                          dsp,
                                          constraints,
                                          false);
  else
    DoFTools::make_sparsity_pattern (dof_handler,
                                     dsp,
                                     constraints,
                                     false);
  
  // setting up dsp with telling communicator and relevant dofs
  SparsityTools::distribute_sparsity_pattern
  (dsp,
   dof_handler.n_locally_owned_dofs_per_processor (),
   MPI_COMM_WORLD,
   relevant_dofs);
  
  // initialize equation objects
  for (unsigned int i=0; i<equ_ptrs.size(); ++i)
  {
    // initialize cell iterators
    equ_ptrs[i]->initialize_cell_iterators_this_proc (msh_ptr, dof_handler);
    // initialize assembly objects
    equ_ptrs[i]->initialize_assembly_related_objects (fe);
    // initialize system matrices and vectors
    equ_ptrs[i]->initialize_system_matrices_vectors (dsp, local_dofs, sflxes_proc);
  }
}

template <int dim>
void BartDriver<dim>::output_results () const
{
  std::string sec_name = "Graphical output";
  DataOut<dim> data_out;
  data_out.attach_dof_handler (dof_handler);

  //if (is_eigen_problem)
  //  data_out.add_data_vector (keff, "keff");
  
  for (unsigned int g=0; g<n_group; ++g)
  {
    std::ostringstream os;
    os << "phi_g_" << g;
    data_out.add_data_vector (sflxes_proc[g], os.str ());
  }

  Vector<float> subdomain (triangulation.n_active_cells ());
  for (unsigned int i=0; i<subdomain.size(); ++i)
    subdomain(i) = triangulation.locally_owned_subdomain ();
  data_out.add_data_vector (subdomain, "subdomain");
  data_out.build_patches ();

  const std::string filename =
  (namebase + "-" + discretization + "-" + Utilities::int_to_string
   (triangulation.locally_owned_subdomain (), 4));
  std::ofstream output ((filename + ".vtu").c_str ());
  data_out.write_vtu (output);

  if (Utilities::MPI::this_mpi_process(MPI_COMM_WORLD)==0)
  {
    std::vector<std::string> filenames;
    for (unsigned int i=0;
         i<Utilities::MPI::n_mpi_processes(MPI_COMM_WORLD);
         ++i)
      filenames.push_back (namebase + "-" + discretization + "-" +
                           Utilities::int_to_string (i) + ".vtu");
    std::ostringstream os;
    os << namebase << "-" << discretization << "-" << global_refinements << ".pvtu";
    std::ofstream master_output ((os.str()).c_str ());
    data_out.write_pvtu_record (master_output, filenames);
  }
}

template <int dim>
void BartDriver<dim>::run ()
{
  msh_ptr->make_grid (triangulation);
  setup_system ();
  report_system ();
  // solve the problem using iterative methods specified in Iterations class
  itr_ptr->solve_problems (sflxes_proc, equ_ptrs, ig_ptr, mg_ptr, eig_ptr);
  if (is_eigen_problem)
    itr_ptr->get_keff (keff);
  output_results ();
}

// explicit instantiation to avoid linking error
template class BartDriver<2>;
template class BartDriver<3>;
