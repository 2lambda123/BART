#include <deal.II/fe/fe_dgq.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/base/utilities.h>
#include <deal.II/base/mpi.h>

#include "bart_tools.h"
#include "../equations/even_parity.h"
#include "../aqdata/aq_lsgc"

template <int dim>
FE_Poly<TensorProductPolynomials<dim>,dim,dim>* build_finite_element
(ParameterHandler &prm)
{
  std::string discretization = prm.get ("spatial discretization");
  AssertThrow (discretization!="none",
               ExcMessage("valid spatial discretizations are dfem and cfem"))
  if (discretization=="dfem")
    return new FE_DGQ<dim> (p_order);
  else
    return new FE_Q<dim> (p_order);
}

template <int dim>
std_cxx11::shared_ptr<PreconditionerSolver> build_linalg (ParameterHandler &prm)
{
  return std_cxx11::shared_ptr<PreconditionerSolver> (new PreconditionerSolver(prm));
}

template <int dim>
std_cxx11::shared_ptr<MeshGenerator<dim> > build_mesh (ParameterHandler &prm)
{
  std_cxx11::shared_ptr<MeshGenerator<dim> > mesh_class =
  std_cxx11::shared_ptr<MeshGenerator<dim> > (new MeshGenerator<dim>(prm));
  return mesh_class;
}

std_cxx11::shared_ptr<MaterialProperties> build_material (ParameterHandler &prm)
{
  std_cxx11::shared_ptr<MaterialProperties> material_class =
  std_cxx11::shared_ptr<MaterialProperties> (new MaterialProperties (prm));
  return material_class;
}

template <int dim>
std_cxx11::shared_ptr<EquationBase<dim> > build_transport_model
(ParameterHandler &prm,
 const std_cxx11::shared_ptr<MeshGenerator<dim> > msh_ptr,
 const std_cxx11::shared_ptr<AQBase<dim> > aqd_ptr,
 const std_cxx11::shared_ptr<MaterialProperties> mat_ptr)
{
  std::string transport_model_name = prm.get ("transport model");
  AssertThrow (transport_model_name!="none",
               ExcMessage("transport model name incorrect or missing"));
  std_cxx11::shared_ptr<EquationBase<dim> > transport_class;
  if (transport_model_name=="ep")
    equation_class =
    std_cxx11::shared_ptr<EquationBase<dim> >
    (new EvenParity<dim> (prm, msh_ptr, aqd_ptr, mat_ptr));
  return equation_class;
}

template <int dim>
std_cxx11::shared_ptr<AQBase<dim> >
build_aq_model (ParameterHandler &prm)
{
  std::string aq_name = prm.get ("angular quadrature name");
  AssertThrow (aq_name!="none",
               ExcMessage("angular quadrature name incorrect or missing"));
  std_cxx11::shared_ptr<AQBase<dim> > aq_class;
  if (aq_name=="lsgc")
    aq_class = std_cxx11::shared_ptr<AQBase<dim> > (new LSGC<dim>(prm));
  return aq_class;
}

void radio (std::string str)
{
  pout << str << std::endl;
}

void radio (std::string str1, std::string str2)
{
  pout << str1 << ": " << str2 << std::endl;
}

void radio (std::string str,
            double num)
{
  pout << str << ": " << num << std::endl;
}

void radio (std::string str1, unsigned int num1,
            std::string str2, unsigned int num2,
            std::string str3, unsigned int num3)
{
  if (Utilities::MPI::this_mpi_process(MPI_COMM_WORLD)==0)
  {
    std::cout << str1 << ": " << num1 << ", ";
    std::cout << str2 << ": " << num2 << ", ";
    std::cout << str3 << ": " << num3 << std::endl;
  }
}

void radio (std::string str, unsigned int num)
{
  pout << str << ": " << num << std::endl;
}

void radio (std::string str, bool boolean)
{
  pout << str << ": " << (boolean?"true":"false") << std::endl;
}

void radio ()
{
  pout << "-------------------------------------" << std::endl << std::endl;
}

template std_cxx11::shared_ptr<EquationBase<2> > build_transport_model;
template std_cxx11::shared_ptr<EquationBase<3> > build_transport_model;
template std_cxx11::shared_ptr<AQBase<2> > build_aq_model;
template std_cxx11::shared_ptr<AQBase<3> > build_aq_model;
template std_cxx11::shared_ptr<MeshGenerator<2> > build_mesh;
template std_cxx11::shared_ptr<MeshGenerator<3> > build_mesh;
