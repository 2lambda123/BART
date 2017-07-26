#include <deal.II/base/utilities.h>

#include "bart_builder.h"
#include "../transport/even_parity.h"
#include "../aqdata/aq_lsgc"

template <int dim>
std_cxx11::shared_ptr<TransportBase<dim> >
build_transport_model (ParameterHandler &prm)
{
  std::string transport_model_name = prm.get ("transport model");
  AssertThrow (transport_model_name!="none",
               ExcMessage("transport model name incorrect or missing"));
  std_cxx11::shared_ptr<TransportBase<dim> > transport_class;
  if (transport_model_name=="ep")
    transport_class =
    std_cxx11::shared_ptr<TransportBase<dim> > (new EvenParity<dim> (prm));
  return transport_class;
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
  if (Utilities::MPI::this_mpi_process(MPI_COMM_WORLD)==0)
    std::cout << str << std::endl;
}

void radio (std::string str1, std::string str2)
{
  if (Utilities::MPI::this_mpi_process(MPI_COMM_WORLD)==0)
    std::cout << str1 << ": " << str2 << std::endl;
}

void radio (std::string str,
            double num)
{
  if (Utilities::MPI::this_mpi_process(MPI_COMM_WORLD)==0)
    std::cout << str << ": " << num << std::endl;
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
  if (Utilities::MPI::this_mpi_process(MPI_COMM_WORLD)==0)
    std::cout << str << ": " << num << std::endl;
}

void radio (std::string str, bool boolean)
{
  if (Utilities::MPI::this_mpi_process(MPI_COMM_WORLD)==0)
    std::cout << str << ": " << (boolean?"true":"false") << std::endl;
}

void radio ()
{
  if (Utilities::MPI::this_mpi_process(MPI_COMM_WORLD)==0)
    std::cout << "-------------------------------------" << std::endl << std::endl;
}

template std_cxx11::shared_ptr<TransportBase<2> > build_transport_model;
template std_cxx11::shared_ptr<TransportBase<3> > build_transport_model;
template std_cxx11::shared_ptr<AQBase<2> > build_aq_model;
template std_cxx11::shared_ptr<AQBase<3> > build_aq_model;

