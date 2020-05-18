#include "formulation/updater/saaf_updater.h"

namespace bart {

namespace formulation {

namespace updater {

template<int dim>
SAAFUpdater<dim>::SAAFUpdater(
    std::unique_ptr<SAAFFormulationType> formulation_ptr,
    std::unique_ptr<StamperType> stamper_ptr,
    const std::shared_ptr<QuadratureSetType>& quadrature_set_ptr)
    : formulation_ptr_(std::move(formulation_ptr)),
      stamper_ptr_(std::move(stamper_ptr)),
      quadrature_set_ptr_(quadrature_set_ptr) {
  AssertThrow(formulation_ptr_ != nullptr,
      dealii::ExcMessage("Error in constructor of SAAFUpdater, formulation "
                         "pointer passed is null"))
  AssertThrow(stamper_ptr_ != nullptr,
              dealii::ExcMessage("Error in constructor of SAAFUpdater, stamper "
                                 "pointer passed is null"))
  AssertThrow(quadrature_set_ptr_ != nullptr,
              dealii::ExcMessage("Error in constructor of SAAFUpdater, "
                                 "quadrature set pointer passed is null"))
}

template<int dim>
SAAFUpdater<dim>::SAAFUpdater(
    std::unique_ptr<SAAFFormulationType> formulation_ptr,
    std::unique_ptr<StamperType> stamper_ptr,
    const std::shared_ptr<QuadratureSetType>& quadrature_set_ptr,
    const EnergyGroupToAngularSolutionPtrMap& angular_solution_ptr_map,
    const std::unordered_set<Boundary> reflective_boundaries)
    : SAAFUpdater(std::move(formulation_ptr), std::move(stamper_ptr),
                  quadrature_set_ptr) {
  reflective_boundaries_ = reflective_boundaries;
  angular_solution_ptr_map_ = angular_solution_ptr_map;
  for (const int total_angles = quadrature_set_ptr_->size();
       auto& [energy_group, solution_ptr] : angular_solution_ptr_map_) {
    AssertThrow(total_angles == solution_ptr->total_angles(),
                dealii::ExcMessage("Error in construction of SAAF Updater, "
                                   "total angles in quadrature set does not "
                                   "match size of one or more angular "
                                   "solutions"));
  }
}

template<int dim>
void SAAFUpdater<dim>::UpdateFixedTerms(
    system::System &to_update,
    system::EnergyGroup group,
    quadrature::QuadraturePointIndex index) {
  auto fixed_matrix_ptr =
      to_update.left_hand_side_ptr_->GetFixedTermPtr({group.get(), index.get()});
  auto quadrature_point_ptr = quadrature_set_ptr_->GetQuadraturePoint(index);
  auto streaming_term_function =
      [&](formulation::FullMatrix& cell_matrix,
          const domain::CellPtr<dim>& cell_ptr) -> void {
        formulation_ptr_->FillCellStreamingTerm(cell_matrix, cell_ptr,
                                                quadrature_point_ptr, group);
      };
  auto collision_term_function =
      [&](formulation::FullMatrix& cell_matrix,
          const domain::CellPtr<dim>& cell_ptr) -> void {
        formulation_ptr_->FillCellCollisionTerm(cell_matrix, cell_ptr, group);
      };
  auto boundary_bilinear_term_function =
      [&](formulation::FullMatrix& cell_matrix,
          const domain::FaceIndex face_index,
          const domain::CellPtr<dim>& cell_ptr) -> void {
    formulation_ptr_->FillBoundaryBilinearTerm(cell_matrix, cell_ptr, face_index, quadrature_point_ptr, group);
  };
  *fixed_matrix_ptr = 0;
  stamper_ptr_->StampMatrix(*fixed_matrix_ptr, streaming_term_function);
  stamper_ptr_->StampMatrix(*fixed_matrix_ptr, collision_term_function);
  stamper_ptr_->StampBoundaryMatrix(*fixed_matrix_ptr,
                                    boundary_bilinear_term_function);
}

template<int dim>
void SAAFUpdater<dim>::UpdateFissionSource(system::System &to_update,
                                           system::EnergyGroup group,
                                           quadrature::QuadraturePointIndex index) {
  auto fission_source_ptr =
      to_update.right_hand_side_ptr_->GetVariableTermPtr({group.get(), index.get()},
                                                         system::terms::VariableLinearTerms::kFissionSource);
  auto quadrature_point_ptr = quadrature_set_ptr_->GetQuadraturePoint(index);
  const auto& current_moments = to_update.current_moments->moments();
  const auto& in_group_moment = current_moments.at({group.get(), 0, 0});
  auto fission_source_function =
      [&](formulation::Vector& cell_vector,
          const domain::CellPtr<dim> &cell_ptr) -> void {
        formulation_ptr_->FillCellFissionSourceTerm(cell_vector,
                                                    cell_ptr,
                                                    quadrature_point_ptr,
                                                    group,
                                                    to_update.k_effective.value(),
                                                    in_group_moment,
                                                    current_moments);
      };
  *fission_source_ptr = 0;
  stamper_ptr_->StampVector(*fission_source_ptr, fission_source_function);
}

template<int dim>
void SAAFUpdater<dim>::UpdateScatteringSource(
    system::System &to_update,
    system::EnergyGroup group,
    quadrature::QuadraturePointIndex index) {
  auto scattering_source_ptr =
      to_update.right_hand_side_ptr_->GetVariableTermPtr({group.get(), index.get()},
                                                         system::terms::VariableLinearTerms::kScatteringSource);
  *scattering_source_ptr = 0;
  auto quadrature_point_ptr = quadrature_set_ptr_->GetQuadraturePoint(index);
  const auto& current_moments = to_update.current_moments->moments();
  const auto& in_group_moment = current_moments.at({group.get(), 0, 0});
  auto scattering_source_function =
      [&](formulation::Vector& cell_vector,
          const domain::CellPtr<dim> &cell_ptr) -> void {
    formulation_ptr_->FillCellScatteringSourceTerm(cell_vector,
                                                   cell_ptr,
                                                   quadrature_point_ptr,
                                                   group,
                                                   in_group_moment,
                                                   current_moments);
  };
  stamper_ptr_->StampVector(*scattering_source_ptr, scattering_source_function);
}

template class SAAFUpdater<1>;
template class SAAFUpdater<2>;
template class SAAFUpdater<3>;

} // namespace updater

} // namespace formulation

} // namespace bart
