#include "formulation/cfem_saaf_stamper.h"

namespace bart {

namespace formulation {

template<int dim>
CFEM_SAAF_Stamper<dim>::CFEM_SAAF_Stamper(
    std::unique_ptr<SAAFFormulationType> saaf_ptr,
    std::shared_ptr<DomainDefinitionType> defintion_ptr)
    : formulation_ptr_(std::move(saaf_ptr)),
      definition_ptr_(defintion_ptr) {

  cells_ = definition_ptr_->Cells();
  saaf_initialization_token_ = formulation_ptr_->Initialize(cells_.at(0));
}

template<int dim>
void CFEM_SAAF_Stamper<dim>::StampBoundaryBilinearTerm(
    system::MPISparseMatrix &to_stamp,
    const std::shared_ptr<quadrature::QuadraturePointI<dim>> quadrature_point,
    const system::EnergyGroup group_number) {

  int faces_per_cell = dealii::GeometryInfo<dim>::faces_per_cell;
  auto boundary_function =
      [&, faces_per_cell](dealii::FullMatrix<double>& matrix,
          const domain::CellPtr<dim>& cell_ptr) -> void {
        if (cell_ptr->at_boundary()) {
          for (int face = 0; face < faces_per_cell; ++face) {
            if (cell_ptr->face(face)->at_boundary()) {

              formulation_ptr_->FillBoundaryBilinearTerm(matrix,
                                               saaf_initialization_token_,
                                               cell_ptr,
                                               domain::FaceIndex(face),
                                               quadrature_point,
                                               group_number);
            }
          }
        }
      };
  StampMatrix(to_stamp, boundary_function);
}


template<int dim>
void CFEM_SAAF_Stamper<dim>::StampCollisionTerm(
    system::MPISparseMatrix &to_stamp,
    const system::EnergyGroup group_number) {
  auto collision_term_function =
      [&](formulation::FullMatrix& cell_matrix,
          const domain::CellPtr<dim>& cell_ptr) -> void {
        formulation_ptr_->FillCellCollisionTerm(cell_matrix,
                                                saaf_initialization_token_,
                                                cell_ptr,
                                                group_number);
  };

  StampMatrix(to_stamp, collision_term_function);
}

template <int dim>
void CFEM_SAAF_Stamper<dim>::StampFissionSourceTerm(
    system::MPIVector& to_stamp,
    const std::shared_ptr<quadrature::QuadraturePointI<dim>> quadrature_point,
    const system::EnergyGroup group_number,
    const double k_eff,
    const system::moments::MomentVector &in_group_moment,
    const system::moments::MomentsMap &group_moments) {
  auto fission_source_function =
      [&](dealii::Vector<double> &vector,
          const domain::CellPtr<dim>& cell_ptr) -> void {
    this->formulation_ptr_->FillCellFissionSourceTerm(
        vector, this->saaf_initialization_token_, cell_ptr, quadrature_point,
        group_number, k_eff, in_group_moment, group_moments);
  };

  StampVector(to_stamp, fission_source_function);
}

template<int dim>
void CFEM_SAAF_Stamper<dim>::StampFixedSourceTerm(
    system::MPIVector &to_stamp,
    const std::shared_ptr<quadrature::QuadraturePointI<dim> > quadrature_point,
    const system::EnergyGroup group_number) {
  auto fixed_source_function =
      [&](dealii::Vector<double> &vector,
          const domain::CellPtr<dim>& cell_ptr) -> void {
        this->formulation_ptr_->FillCellFixedSourceTerm(
            vector, this->saaf_initialization_token_, cell_ptr,
            quadrature_point, group_number);
  };

  StampVector(to_stamp, fixed_source_function);
}

template<int dim>
void CFEM_SAAF_Stamper<dim>::StampScatteringSourceTerm(
    bart::system::MPIVector &to_stamp,
    const std::shared_ptr<bart::quadrature::QuadraturePointI<dim>> quadrature_point,
    const bart::system::EnergyGroup group_number,
    const bart::system::moments::MomentVector &in_group_moment,
    const bart::system::moments::MomentsMap &group_moments) {
  auto scattering_source_function =
      [&](dealii::Vector<double> &vector,
          const domain::CellPtr<dim>& cell_ptr) -> void {
        this->formulation_ptr_->FillCellScatteringSourceTerm(
            vector, this->saaf_initialization_token_, cell_ptr, quadrature_point,
            group_number, in_group_moment, group_moments);
      };

  StampVector(to_stamp, scattering_source_function);
}

template<int dim>
void CFEM_SAAF_Stamper<dim>::StampStreamingTerm(
    system::MPISparseMatrix &to_stamp,
    const std::shared_ptr<quadrature::QuadraturePointI<dim>> quadrature_point,
    const system::EnergyGroup group_number) {
  auto streaming_term_function =
      [&](formulation::FullMatrix& cell_matrix,
          const domain::CellPtr<dim>& cell_ptr) -> void {
        formulation_ptr_->FillCellStreamingTerm(cell_matrix,
                                                saaf_initialization_token_,
                                                cell_ptr,
                                                quadrature_point,
                                                group_number);
      };

  StampMatrix(to_stamp, streaming_term_function);
}


template<int dim>
void CFEM_SAAF_Stamper<dim>::StampMatrix(
    system::MPISparseMatrix &to_stamp,
    std::function<void(formulation::FullMatrix&,
                       const domain::CellPtr<dim> &)> stamping_function) {
  auto cell_matrix = definition_ptr_->GetCellMatrix();
  std::vector<dealii::types::global_dof_index>
      local_dof_indices(cell_matrix.n_cols());

  for (const auto& cell : cells_) {
    cell_matrix = 0;
    cell->get_dof_indices(local_dof_indices);
    stamping_function(cell_matrix, cell);
    to_stamp.add(local_dof_indices, local_dof_indices, cell_matrix);
  }
  to_stamp.compress(dealii::VectorOperation::add);
}

template <int dim>
void CFEM_SAAF_Stamper<dim>::StampVector(
    bart::system::MPIVector &to_stamp,
    std::function<void(
        bart::formulation::Vector &,
        const bart::domain::CellPtr<dim> &)> stamping_function) {
  auto cell_vector = definition_ptr_->GetCellVector();
  std::vector<dealii::types::global_dof_index> local_dof_indices(cell_vector.size());

  for (const auto& cell : cells_) {
    cell_vector = 0;
    cell->get_dof_indices(local_dof_indices);
    stamping_function(cell_vector, cell);
    to_stamp.add(local_dof_indices, cell_vector);
  }
  to_stamp.compress(dealii::VectorOperation::add);
}

template class CFEM_SAAF_Stamper<1>;
template class CFEM_SAAF_Stamper<2>;
template class CFEM_SAAF_Stamper<3>;

} // namespace formulation

} // namespace bart
