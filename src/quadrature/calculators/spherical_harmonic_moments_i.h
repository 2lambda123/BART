#ifndef BART_SRC_QUADRATURE_CALCULATORS_SPHERICAL_HARMONIC_MOMENTS_I_H_
#define BART_SRC_QUADRATURE_CALCULATORS_SPHERICAL_HARMONIC_MOMENTS_I_H_

#include "system/moments/spherical_harmonic_types.h"
#include "system/system_types.h"

namespace bart {

namespace system {

namespace solution {

class MPIGroupAngularSolutionI;

} // namespace solution

} // namespace system

namespace quadrature {

namespace angular {

template <int dim>
class AngularQuadratureSetI;

} // namespace angular

namespace calculators {

/*! \brief Interface for classes that calculate spherical harmonic moments.
 *
 * This class provides the interface for classes that will take a solution
 * and calculate the spherical harmonic moments of that solution. The spherical
 * harmonic moment for group \f$g\f$, degree \f$\ell$\f$ and order \f$m\f$ is
 * given by:
 *
 * \f[
 *
 * \phi_{g}^{\ell, m}(\vec{r}) = \sum_{i = 0}^{N}w_i\psi_{g}(\vec{r}, \hat{\Omega}_i)Y_{\ell}^{m}(\hat{\Omega}_i)\;.
 *
 * \f]
 *
 * The angular quadrature is provided as a dependency to this class, which
 * provides the weights and angles for the calculation. The solutions \f$\psi\f$
 * are provided by a class derived from system::solution::MPIAngular, and must
 * have solutions at each angle \f$\hat{\Omega}\f$. This is ensured by solving
 * using collocation at each angle.
 *
 *
 * @tparam dim angular dimension of the calculator, based on the angular
 *         dimension of the angular quadrature set.
 */
template <int dim>
class SphericalHarmonicMomentsI {
 public:
  virtual ~SphericalHarmonicMomentsI() = default;

  virtual system::moments::MomentVector CalculateMoment(
      system::solution::MPIGroupAngularSolutionI* solution,
      system::GroupNumber group,
      system::moments::HarmonicL harmonic_l,
      system::moments::HarmonicL harmonic_m) const = 0;

  virtual angular::AngularQuadratureSetI<dim>* angular_quadrature_set_ptr() const = 0;
};

} // namespace calculators

} // namespace quadrature

} // namespace bart

#endif // BART_SRC_QUADRATURE_CALCULATORS_SPHERICAL_HARMONIC_MOMENTS_I_H_