#include "calculator/cell/fission_source_norm.h"

#include "data/cross_sections.h"
#include "domain/finite_element_i.h"
#include "system/moments/spherical_harmonic_i.h"

namespace bart {

namespace calculator {

namespace cell {

template<int dim>
FissionSourceNorm<dim>::FissionSourceNorm(
    std::shared_ptr<domain::FiniteElementI<dim>> finite_element_ptr,
    std::shared_ptr<data::CrossSections> cross_sections_ptr)
    : finite_element_ptr_(finite_element_ptr),
      cross_sections_ptr_(cross_sections_ptr),
      cell_quadrature_points_(finite_element_ptr->n_cell_quad_pts()) {}


template<int dim>
double FissionSourceNorm<dim>::GetCellNorm(
    domain::CellPtr<dim> cell_ptr,
    system::moments::SphericalHarmonicI* system_moments) const {

  const int material_id = cell_ptr->material_id();
  double fission_source = 0;

  if (cross_sections_ptr_->is_material_fissile.at(material_id)) {
    finite_element_ptr_->SetCell(cell_ptr);
    fission_source = -1;
  }

  return fission_source;
}

template class FissionSourceNorm<1>;
template class FissionSourceNorm<2>;
template class FissionSourceNorm<3>;

} // namespace cell

} // namespace calculator

} // namespace bart