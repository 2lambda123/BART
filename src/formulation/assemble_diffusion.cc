#include "formulation/assemble_diffusion.h"

#include "formulation/types.h"

namespace bart {

namespace formulation {

template <int dim>
AssembleDiffusion<dim>::AssembleDiffusion(
    std::unique_ptr<equation::Diffusion<dim>> equation,
    std::unique_ptr<domain::Definition<dim>> domain,
    std::shared_ptr<data::ScalarSystemMatrixPtrs> system_matrix_ptrs,
    std::shared_ptr<data::ScalarRightHandSidePtrs> right_hand_side_ptrs,
    std::unique_ptr<data::ScalarRightHandSidePtrs> fixed_right_hand_side_ptrs,
    std::map<problem::Boundary, bool> reflective_boundary_map)
    : equation_(std::move(equation)),
      domain_(std::move(domain)),
      system_matrix_ptrs_(system_matrix_ptrs),
      right_hand_side_ptrs_(right_hand_side_ptrs),
      fixed_right_hand_side_ptrs_(std::move(fixed_right_hand_side_ptrs)),
      reflective_boundary_map_(reflective_boundary_map) {
  equation_->Precalculate(domain_->Cells()[0]);
  for (auto &rhs : *fixed_right_hand_side_ptrs_) {
    *rhs.second = 0.0;
  }
}

template <int dim>
void AssembleDiffusion<dim>::AssembleFixedBilinearTerms(const GroupNumber group) {

  CellMatrix cell_matrix = domain_->GetCellMatrix();
  std::vector<dealii::types::global_dof_index> local_indices;
  int faces_per_cell = dealii::GeometryInfo<dim>::faces_per_cell;

  auto &system_matrix = *(system_matrix_ptrs_->at(group));

  system_matrix = 0.0;

  for (const auto &cell : domain_->Cells()) {
    cell_matrix = 0.0;
    equation_->FillCellStreamingTerm(cell_matrix, cell, group);
    equation_->FillCellCollisionTerm(cell_matrix, cell, group);

    for (int face_number = 0; face_number < faces_per_cell; ++face_number) {

      if (cell->at_boundary(face_number)) {
        auto boundary_type = BoundaryType::kVacuum;
        auto boundary_name = static_cast<problem::Boundary>(
            cell->face(face_number)->boundary_id());
        bool is_reflective = reflective_boundary_map_[boundary_name];

        if (is_reflective)
          boundary_type = BoundaryType::kReflective;

        equation_->FillBoundaryTerm(cell_matrix, cell, group,
                                    face_number, boundary_type);
      }
    }

    cell->get_dof_indices(local_indices);
    system_matrix.add(local_indices, local_indices, cell_matrix);
  }
  system_matrix.compress(dealii::VectorOperation::add);
}

template<int dim>
void AssembleDiffusion<dim>::AssembleFixedLinearTerms(const GroupNumber group) {
  CellVector cell_vector = domain_->GetCellVector();
  std::vector<dealii::types::global_dof_index> local_indices;

  auto &fixed_rhs = *(fixed_right_hand_side_ptrs_->at(group));

  for (const auto &cell : domain_->Cells()) {
    cell_vector = 0.0;
    cell->get_dof_indices(local_indices);
    equation_->FillCellFixedSource(cell_vector, cell, group);
    fixed_rhs.add(local_indices, cell_vector);
  }

  fixed_rhs.compress(dealii::VectorOperation::add);
}

template <int dim>
void AssembleDiffusion<dim>::AssembleFissionTerm(
    const GroupNumber group,
    const double k_effective,
    const data::ScalarFluxPtrs &scalar_flux_ptrs) {

  CellVector cell_vector = domain_->GetCellVector();
  std::vector<dealii::types::global_dof_index> local_indices;

  auto &rhs = *(right_hand_side_ptrs_->at(group));

  for (const auto &cell : domain_->Cells()) {
    cell_vector = 0.0;
    cell->get_dof_indices(local_indices);
    equation_->FillCellFissionSource(cell_vector, cell, group,
                                     k_effective, scalar_flux_ptrs);
    rhs.add(local_indices, cell_vector);
  }

  rhs.compress(dealii::VectorOperation::add);
}

template <int dim>
void AssembleDiffusion<dim>::AssembleScatteringSourceTerm(
    const GroupNumber group,
    const data::ScalarFluxPtrs &scalar_flux_ptrs) {

  CellVector cell_vector = domain_->GetCellVector();
  std::vector<dealii::types::global_dof_index> local_indices;

  auto &rhs = *(right_hand_side_ptrs_->at(group));

  for (const auto &cell : domain_->Cells()) {
    cell_vector = 0.0;
    cell->get_dof_indices(local_indices);
    equation_->FillCellScatteringSource(cell_vector, cell, group,
                                        scalar_flux_ptrs);
    rhs.add(local_indices, cell_vector);
  }

  rhs.compress(dealii::VectorOperation::add);
}


template class AssembleDiffusion<1>;
template class AssembleDiffusion<2>;
template class AssembleDiffusion<3>;

} // namespace formulation

} // namespace bart

