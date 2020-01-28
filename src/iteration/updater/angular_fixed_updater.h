#ifndef BART_SRC_ITERATION_UPDATER_ANGULAR_FIXED_UPDATER_H_
#define BART_SRC_ITERATION_UPDATER_ANGULAR_FIXED_UPDATER_H_

#include "iteration/updater/fixed_updater_i.h"
#include "quadrature/quadrature_set_i.h"

namespace bart {

namespace iteration {

namespace updater {

template <typename StamperType>
class AngularFixedUpdater : public FixedUpdaterI {
 public:
  static constexpr int dim = StamperType::dimension;

  using QuadratureSetType = quadrature::QuadratureSetI<dim>;

  AngularFixedUpdater(
      std::shared_ptr<StamperType>,
      std::shared_ptr<QuadratureSetType>);

  void UpdateFixedTerms(system::System &system,
                        system::GroupNumber group,
                        system::AngleIndex angle) override {}

  StamperType* stamper_ptr() const { return stamper_ptr_.get();};

 private:
  std::shared_ptr<StamperType> stamper_ptr_ = nullptr;
};

} // namespace updater

} // namespace iteration

} // namespace bart

#endif //BART_SRC_ITERATION_UPDATER_ANGULAR_FIXED_UPDATER_H_
