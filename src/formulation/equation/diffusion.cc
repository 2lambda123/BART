#include "formulation/equation/diffusion.h"
#include "diffusion.h"

#include <vector>



namespace bart {

namespace formulation {

namespace equation {

template<int dim>
void Diffusion<dim>::Precalculate(const CellPtr &cell_ptr) {
  SetCell(cell_ptr);

  for (int q = 0; q < cell_quadrature_points_; ++q) {
    Matrix shape_squared(cell_degrees_of_freedom_,
                         cell_degrees_of_freedom_);
    Matrix gradient_squared(cell_degrees_of_freedom_,
                            cell_degrees_of_freedom_);

    for (int i = 0; i < cell_degrees_of_freedom_; ++i) {
      for (int j = 0; j < cell_degrees_of_freedom_; ++j) {
        shape_squared(i,j) = finite_element_->values()->shape_value(i,q) *
            finite_element_->values()->shape_value(j,q);
        gradient_squared(i,j) = finite_element_->values()->shape_grad(i,q) *
            finite_element_->values()->shape_grad(j,q);
      }
    }

    shape_squared_[q] = shape_squared;
    gradient_squared_[q] = gradient_squared;
  }
}

template<int dim>
void Diffusion<dim>::FillCellBilinearTerm(Matrix &to_fill,
                                          const CellPtr &cell_ptr,
                                          const GroupNumber group) const {
  SetCell(cell_ptr);
  MaterialID material_id = cell_ptr->material_id();

  double sigma_t = cross_sections_->sigma_t.at(material_id)[group];
  double sigma_s = cross_sections_->sigma_s.at(material_id)(group, group);
  double sigma_r = sigma_t - sigma_s;

  double diff_coef = cross_sections_->diffusion_coef.at(material_id)[group];

  for (int q = 0; q < cell_quadrature_points_; ++q) {
    double gradient = finite_element_->values()->JxW(q);
    to_fill.add(diff_coef*gradient, gradient_squared_[q]);
    to_fill.add(sigma_r*gradient, shape_squared_[q]);
  }
}

template<int dim>
void Diffusion<dim>::FillBoundaryBilinearTerm(
    Matrix &to_fill,
    const CellPtr &cell_ptr,
    const GroupNumber,
    const FaceNumber face_number,
    const BoundaryType boundary_type) const {

  if (boundary_type == BoundaryType::kVacuum) {
    SetFace(cell_ptr, face_number);

    for (int q = 0; q < face_quadrature_points_; ++q) {
      for (int i = 0; i < cell_degrees_of_freedom_; ++i) {
        for (int j = 0; j < cell_degrees_of_freedom_; ++j) {
          to_fill(i,j) += (
              finite_element_->face_values()->shape_value(i, q) *
              finite_element_->face_values()->shape_value(j,q)
              ) * 0.5 * finite_element_->face_values()->JxW(q);
        }
      }
    }
  }
}

template<int dim>
void Diffusion<dim>::FillCellLinearFixedTerm(Vector &rhs_to_fill,
                                             const CellPtr &cell_ptr,
                                             const GroupNumber group,
                                             const bool is_eigen_problem) const {
  SetCell(cell_ptr);
  MaterialID material_id = cell_ptr->material_id();

  // Fixed source at each cell quadrature point
  std::vector<double> cell_fixed_source(cell_quadrature_points_);

  if (!is_eigen_problem) {
    cell_fixed_source = std::vector<double>(
        cell_quadrature_points_,
        cross_sections_->q.at(material_id)[group]);
  }
}

template <int dim>
void Diffusion<dim>::FillCellLinearScatteringTerm(Vector &rhs_to_fill,
                                                  const CellPtr &cell_ptr,
                                                  const GroupNumber group) const {
  SetCell(cell_ptr);
  MaterialID material_id = cell_ptr->material_id();
  int total_groups = scalar_fluxes_->previous_iteration.size();

  // Scattering flux at each cell quadrature point, with a contribution from
  // all other groups.
  std::vector<double> cell_scatter_flux(cell_quadrature_points_);

  // Iterates over each group to provide contribution from each group
  for (int group_in = 0; group_in < total_groups; ++group_in) {
    std::vector<double> group_cell_scalar_flux(cell_quadrature_points_);

    if (group_in != group) {
      double sigma_s = cross_sections_->sigma_s.at(material_id)(group_in, group);

      if (sigma_s >= 1e-15) {
        finite_element_->values()->get_function_values(
            *scalar_fluxes_->previous_iteration[group],
            group_cell_scalar_flux);

        for (int q = 0; q < cell_quadrature_points_; ++q) {
          cell_scatter_flux[q] += sigma_s * group_cell_scalar_flux[q];
        }
      }
    }
  }

  for (int q = 0; q < cell_quadrature_points_; ++q) {
    cell_scatter_flux[q] *= finite_element_->values()->JxW(q);
    for (int i = 0; i < cell_degrees_of_freedom_; ++i) {
      rhs_to_fill(i) += finite_element_->values()->shape_value(i, q) *
          cell_scatter_flux[q];
    }
  }

}


template class Diffusion<1>;
template class Diffusion<2>;
template class Diffusion<3>;

} // namespace equation

} // namespace formulation

} // namespace bart