#include "domain.h"

namespace bart {

namespace problem {

template <int dim>
Domain<dim>::Domain(std::unique_ptr<domain::MeshI<dim>> &mesh,
                    std::unique_ptr<domain::FiniteElementI<dim>> &finite_element)
    : mesh_(std::move(mesh)),
      finite_element_(std::move(finite_element)),
      triangulation_(MPI_COMM_WORLD,
                     typename dealii::Triangulation<dim>::MeshSmoothing(
                         dealii::Triangulation<dim>::smoothing_on_refinement |
                         dealii::Triangulation<dim>::smoothing_on_coarsening)),
      dof_handler_(triangulation_)
{}

template class Domain<1>;
template class Domain<2>;

} // namespace bart

} // namespace problem
