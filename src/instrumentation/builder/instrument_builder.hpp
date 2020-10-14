#ifndef BART_SRC_INSTRUMENTATION_BUILDER_INSTRUMENT_BUILDER_HPP_
#define BART_SRC_INSTRUMENTATION_BUILDER_INSTRUMENT_BUILDER_HPP_

#include <memory>

#include "instrumentation/instrument_i.h"

namespace bart::instrumentation::builder {

enum class InstrumentName {
  kColorStatusToConditionalOstream = 0,
  kConvergenceStatusToConditionalOstream = 1,
  kStringToConditionalOstream = 2,
};

class InstrumentBuilder {
 public:
  template <typename InputType>
  [[nodiscard]] static auto BuildInstrument(
      const InstrumentName name) -> std::unique_ptr<InstrumentI<InputType>>;
};

} // namespace bart::instrumentation::builder

#endif //BART_SRC_INSTRUMENTATION_BUILDER_INSTRUMENT_BUILDER_HPP_
