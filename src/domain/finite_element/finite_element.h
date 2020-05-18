#ifndef BART_DOMAIN_FINITE_ELEMENT_H_
#define BART_DOMAIN_FINITE_ELEMENT_H_

#include <deal.II/base/quadrature_lib.h>
#include <deal.II/fe/fe_values.h>
#include <system/system_types.h>

#include "domain/finite_element/finite_element_i.h"

namespace bart {

namespace domain {

namespace finite_element {

template<int dim>
class FiniteElement : public FiniteElementI<dim> {
 public:
  virtual ~FiniteElement() = default;

  // Basic Finite Element data
  dealii::FiniteElement<dim, dim> *finite_element() override {
    return finite_element_.get(); };

  int dofs_per_cell() const override { return finite_element_->dofs_per_cell; }

  int n_cell_quad_pts() const override { return cell_quadrature_->size(); }

  int n_face_quad_pts() const override { return face_quadrature_->size(); }

  dealii::FEValues<dim> *values() override { return values_.get(); };

  dealii::FEFaceValues<dim> *face_values() override {
    return face_values_.get(); };

  dealii::FEFaceValues<dim> *neighbor_face_values() override {
    return neighbor_face_values_.get(); };

  dealii::QGauss<dim> *cell_quadrature() { return cell_quadrature_.get(); };

  dealii::QGauss<dim - 1> *face_quadrature() { return face_quadrature_.get(); };


  bool SetCell(const domain::CellPtr<dim> &to_set) override;

  bool SetFace(const domain::CellPtr<dim> &to_set,
               const domain::FaceIndex face) override;

  double ShapeValue(const int cell_degree_of_freedom,
                    const int cell_quadrature_point) const override {
    return values_->shape_value(cell_degree_of_freedom, cell_quadrature_point);
  }

  double FaceShapeValue(const int cell_degree_of_freedom,
                        const int face_quadrature_point) const override {
    return face_values_->shape_value(cell_degree_of_freedom, face_quadrature_point);
  }

  dealii::Tensor<1, dim> ShapeGradient(const int cell_degree_of_freedom,
                                       const int cell_quadrature_point) const override {
    return values_->shape_grad(cell_degree_of_freedom, cell_quadrature_point);
  }

  double Jacobian(const int cell_quadrature_point) const override {
    return values_->JxW(cell_quadrature_point);
  }

  double FaceJacobian(const int face_quadrature_point) const override {
    return face_values_->JxW(face_quadrature_point);
  }

  dealii::Tensor<1, dim> FaceNormal() const override {
    return face_values_->normal_vector(0);
  };

  std::vector<double> ValueAtQuadrature(
      const system::moments::MomentVector moment) const override;

  std::vector<double> ValueAtFaceQuadrature(
      const system::MPIVector &mpi_vector) const override {
    std::vector<double> return_vector(finite_element_->dofs_per_cell, 0);
    face_values_->get_function_values(mpi_vector, return_vector);
    return return_vector;
  }

 protected:
  std::shared_ptr<dealii::FiniteElement<dim, dim>> finite_element_;
  std::shared_ptr<dealii::FEValues<dim>> values_;
  std::shared_ptr<dealii::FEFaceValues<dim>> face_values_;
  std::shared_ptr<dealii::FEFaceValues<dim>> neighbor_face_values_;
  std::shared_ptr<dealii::QGauss<dim>> cell_quadrature_;
  std::shared_ptr<dealii::QGauss<dim - 1>> face_quadrature_;

  bool values_reinit_called_ = false;
  bool face_values_reinit_called_ = false;
  bool neighbor_face_values_reinit_called_ = false;
  using FiniteElementI<dim>::values;
  using FiniteElementI<dim>::face_values;

};

} // namespace finite_element

} // namespace domain

} // namespace bart

#endif // BART_DOMAIN_FINITE_ELEMENT_H_