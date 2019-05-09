#ifndef BART_SRC_ITERATION_UPDATER_SOURCE_UPDATER_H_
#define BART_SRC_ITERATION_UPDATER_SOURCE_UPDATER_H_

#include <memory>

#include "data/system/system_types.h"
#include "iteration/updater/source_updater_i.h"
#include "formulation/cfem_stamper_i.h"

namespace bart {

namespace iteration {

namespace updater {

template <typename StamperType>
class SourceUpdater : public SourceUpdaterI {
 public:
  using VariableTerms = data::system::RightHandSideI::VariableTerms;
  using MPIVector = data::system::MPIVector;

  explicit SourceUpdater(std::unique_ptr<StamperType> stamper_ptr)
      : stamper_ptr_(std::move(stamper_ptr)) {};
  virtual ~SourceUpdater() override = default;

  /*! Returns the right hand side vector from the system for a specific source term.
   *
   * @param term source term
   * @param system system holding the right hand side data
   * @param group right hand side group
   * @param angle right hand side angle
   * @return
   */
  std::shared_ptr<MPIVector> GetSourceVectorPtr(VariableTerms term,
                                                data::System& system,
                                                data::system::GroupNumber group,
                                                data::system::AngleIndex angle);
 protected:
  std::unique_ptr<StamperType> stamper_ptr_;

};

} // namespace updater

} // namespace iteration

} // namespace bart

#endif // BART_SRC_ITERATION_UPDATER_SOURCE_UPDATER_H_