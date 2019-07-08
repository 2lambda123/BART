#include "iteration/updater/source_updater_gauss_seidel.h"

#include <sstream>

#include "system/system.h"

namespace bart {

namespace iteration {

namespace updater {

template <>
void SourceUpdaterGaussSeidel<formulation::CFEMStamperI>::UpdateScatteringSource(
    system::System& system,
    system::GroupNumber group,
    system::AngleIndex angle) {

  auto scattering_source_vector_ptr_ =
      this->GetSourceVectorPtr(VariableTerms::kScatteringSource,
                               system, group, angle);

  *scattering_source_vector_ptr_ = 0;

  const system::moments::MomentVector &in_group_moment =
      system.current_moments->GetMoment({group, 0, 0});

  const system::moments::MomentsMap& out_group_moments =
      system.current_moments->moments();


  stamper_ptr_->StampScatteringSource(*scattering_source_vector_ptr_,
                                      group,
                                      in_group_moment,
                                      out_group_moments);
}

template <>
void SourceUpdaterGaussSeidel<formulation::CFEMStamperI>::UpdateFissionSource(
    system::System& system,
    system::GroupNumber group,
    system::AngleIndex angle) {

    double k_effective;

  if (system.k_effective.has_value()) {
    k_effective = system.k_effective.value();
  } else {
    AssertThrow(false, dealii::ExcMessage("System has no k_effective value"));
  }

  AssertThrow(k_effective > 0, dealii::ExcMessage("Bad k_effective value"));

  auto scattering_source_vector_ptr_ =
      this->GetSourceVectorPtr(VariableTerms::kFissionSource,
                               system, group, angle);


  *scattering_source_vector_ptr_ = 0;

  const system::moments::MomentVector &in_group_moment =
      system.current_moments->GetMoment({group, 0, 0});

  const system::moments::MomentsMap& out_group_moments =
      system.current_moments->moments();

  stamper_ptr_->StampFissionSource(*scattering_source_vector_ptr_,
                                   group,
                                   system.k_effective.value(),
                                   in_group_moment,
                                   out_group_moments);
}

template class SourceUpdaterGaussSeidel<formulation::CFEMStamperI>;

} // namespace updater

} // namespace iteration

} // namespace bart