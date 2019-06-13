#ifndef BART_SRC_SYSTEM_MOMENTS_SPHERICAL_HARMONIC_H_
#define BART_SRC_SYSTEM_MOMENTS_SPHERICAL_HARMONIC_H_

#include "system/moments/spherical_harmonic_types.h"
#include "system/moments/spherical_harmonic_i.h"

namespace bart {

namespace system {

namespace moments {
/*! \brief Stores spherical harmonic moments.
 *
 * This class stores the spherical harmonic moments for a system. Each is stored
 * using a unique index made up of the group, degree, and order of the moment
 * (See the documentation for system::moments::SphericalHarmonicI for the
 * definition of these values). After construction, the value of
 * \f$\ell_{\text{max}}\f$ and \f$g\f$ cannot be changed.
 *
 * The underlying vectors are stored as type dealii::Vector<double> and are
 * constructed but unitialized. The `reinit` function must be called or they
 * must be set equal to an existing vector.
 *
 * \code{cpp}
 * // initialize object with two groups, and l_max = 2
 * system::moments::SphericalHarmonic moments(2, 2);
 *
 *
 *
 * \endcode
 */
class SphericalHarmonic : public SphericalHarmonicI {
 public:
  SphericalHarmonic(const int total_groups,
                    const int max_harmonic_l)
      : total_groups_(total_groups),
        max_harmonic_l_(max_harmonic_l) {};
  virtual ~SphericalHarmonic() = default;

  int total_groups() const override { return total_groups_; }
  int max_harmonic_l() const override { return max_harmonic_l_;}
 private:
  const int total_groups_ = 0;
  const int max_harmonic_l_ = 0;
};

} // namespace moments

} // namespace system

} // namespace bart

#endif // BART_SRC_SYSTEM_MOMENTS_SPHERICAL_HARMONIC_H_