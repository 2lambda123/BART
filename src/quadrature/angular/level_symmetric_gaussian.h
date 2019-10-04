#ifndef BART_SRC_QUADRATURE_ANGULAR_LEVEL_SYMMETRIC_GAUSSIAN_H_
#define BART_SRC_QUADRATURE_ANGULAR_LEVEL_SYMMETRIC_GAUSSIAN_H_

#include "quadrature/quadrature_generator_i.h"

namespace bart {

namespace quadrature {

namespace angular {
/*! \brief Generates a level-symmetric-like product Gaussian quadrature set.
 *
 * A product Gaussian quadrature set in spherical coordinates integrates a
 * function \f$f(\theta, \phi)\f$ using the approximation,
 *
 * \f[
 * \int_{0}^{2\pi}\int_{0}^{\pi}d f(\theta, \phi) d\phi \sin{\theta}d\theta
 * \approx \frac{\pi}{m}\sum_{j = 1}^{2m}\sum_{i = 1}^{m}w_i f(\theta_i, \phi_j)\;,
 *
 * \f]
 *
 * where \f$\cos{\theta_i}\f$ and \f$w_i\f$ are chosen to be the Gauss-Legendre nodes and
 * \f$\phi_j\f$ are equally spaced on \f$[0, 2\pi]\f$. This creates the
 * standard product Gaussian quadrature set. As you can see there are many points
 * grouped near the pole, and it is therefore not rotationally symmetric.
 *
 * \image html product_gaussian.png
 *
 * We can make a level-symmetric-like set that is rotationally symmetrical by
 * adapting the number of points in each \f$\phi\f$ level. This set looks like:
 *
 * \image html level_symmetric.png
 *
 * For a set of order \f$n\f$ there are \f$n/2\f$ levels with increasing points
 * per level. The weight therefore changes with each, giving us the following
 * quadrature formula:
 *  \f[
 *
 * I(f) = \pi\sum_{\ell=1}^{n/2}
 * \frac{1}{2\ell}\sum_{j = 1}^{\ell}w_{\ell} f(\theta_{\ell}, \phi_j)\;,
 *
 * \f]
 *
 * where we are only generating the points in the first quadrant. To do this, we
 * use a Gaussian quadrature with \f$n\f$ points, and use the first half of them
 * to get the values of \f$\mu_{\ell}\f$ for each level. Then, generate the
 * values of \f$\phi_{j} = \left(j + 0.5\right)\frac{\pi}{2\ell}\f$, with equal
 * weights on each level equal to \f$w_{\ell}\frac{2\pi}{\ell}\f$.
 *
 * \note Those that dig into the code will notice an extra factor of 2 in the
 *       equation for the weight. This is because the quadrature generated by
 *       dealii extends from 0 to 1, and is extended from -1 to 1, requiring an
 *       additional factor of 2.
 *
 *
 */
class LevelSymmetricGaussian : QuadratureGeneratorI<3> {
 public:
  explicit LevelSymmetricGaussian(quadrature::Order);
  std::vector<std::pair<CartesianPosition<3>, Weight>> GenerateSet() const;

  int order() const {
    return order_;
  }

 private:
  const int order_ = 0;
};


} // namespace angular

} // namespace quadrature

} //namespace bart

#endif //BART_SRC_QUADRATURE_ANGULAR_LEVEL_SYMMETRIC_GAUSSIAN_H_
