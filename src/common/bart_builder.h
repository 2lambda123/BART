#ifndef BART_SRC_COMMON_BART_BUILDER_H_
#define BART_SRC_COMMON_BART_BUILDER_H_

// aq data
#include "../aqdata/aq_base.h"
#include "../aqdata/lsgc.h"

#include <vector>

#include <deal.II/base/parameter_handler.h>
#include <deal.II/fe/fe.h>

//! This namespace provides builders to build objects in BART.
/*!
 This namespace provides functionalities to build objects used in BART, e.g. FE
 spaces. The motivation is s.t. builder functions originally living in
 BARTDriver will be separated to increase the clearness. And it will also
 benifit the unit testing.
 \author Weixiong Zheng
 \date 2018/03
 */
namespace bbuilders {
  //! Function used to build FE spaces for transport equations.
  /*!
   The main functionality is to produce finite element spaces for transport
   equation and NDA if required based on parameters specified in prm.

   \param prm dealii::ParameterHandler object.
   \return A Hash table containing pointers of FE spaces.
   */
  template <int dim>
  std::unordered_map<std::string, dealii::FiniteElement<dim, dim>*>
      BuildFESpaces (const dealii::ParameterHandler &prm);

  //! Function used to build AQ data.
  /*!
   The main functionality is to build angular quadrature data based on parameters
   specified in prm. For 1D, Gauss-Legendre quadrature will be built while in
   multi-D, AQBase will be cast to specific model.

   \param prm dealii::ParameterHandler object.
   \param aq_ptr Angular quadrature pointer that needs to be built.
   \return Void.
   */
  template <int dim>
  void BuildAQ (const dealii::ParameterHandler &prm,
      std::unique_ptr<AQBase<dim>> &aq_ptr);

  /*!
   The same as previous function but returns the pointer to AQ instead.

   \param prm dealii::ParameterHandler object.
   \return Angular quadrature pointer that needs to be built.
   */
  template <int dim>
  std::unique_ptr<AQBase<dim>> BuildAQ (const dealii::ParameterHandler &prm);

  //! Function used to build material
  /*!
   The main functionality is to build pointer to object of MaterialProperties
   based on parameters specified in prm.

   \param prm dealii::ParameterHandler object.
   \param mat_ptr MaterialProperties object pointer.
   \return Void.
   */
  void BuildMaterial (dealii::ParameterHandler &prm,
      std::unique_ptr<MaterialProperties> &mat_ptr);

  //! Function used to build material
  /*!
   The same as previous function but returning pointer to material.

   \param prm dealii::ParameterHandler object.
   \return MaterialProperties object pointer.
   */
  std::unique_ptr<MaterialProperties> BuildMaterial (
      dealii::ParameterHandler &prm);

  //! Function used to build mesh
  /*!
   The main functionality is to build pointer to object of MeshGenerator<dim>
   based on parameters specified in prm.

   \param prm dealii::ParameterHandler object.
   \param msh_ptr MeshGenerator object pointer.
   \return Void.
   */
  template <int dim>
  void BuildMesh (dealii::ParameterHandler &prm,
      std::unique_ptr<MeshGenerator<dim>> &msh_ptr);

  /*!
   The same as previous function but returning mesh pointer.

   \param prm dealii::ParameterHandler object.
   \return MeshGenerator object pointer.
   */
  template <int dim>
  std::unique_ptr<MeshGenerator<dim>> BuildMesh (
      dealii::ParameterHandler &prm);
};

#endif // BART_SRC_COMMON_BART_BUILDER_H_
