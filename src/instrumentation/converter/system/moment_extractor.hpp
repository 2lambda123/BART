#ifndef BART_SRC_INSTRUMENTATION_CONVERTER_SYSTEM_MOMENT_EXTRACTOR_HPP_
#define BART_SRC_INSTRUMENTATION_CONVERTER_SYSTEM_MOMENT_EXTRACTOR_HPP_

#include "instrumentation/converter/converter_i.h"

#include "system/moments/spherical_harmonic_i.h"

namespace bart::instrumentation::converter::system {

namespace moments = bart::system::moments;

/*! \brief Extracts a moment vector from a collection. */
class MomentExtractor :
    public ConverterI<moments::SphericalHarmonicI,
                      moments::MomentVector> {
 public:
  explicit MomentExtractor(const int group_to_extract)
      : group_to_extract_(group_to_extract) {};
  moments::MomentVector Convert(
      const moments::SphericalHarmonicI &input) const override;

  constexpr int group_to_extract() const noexcept { return group_to_extract_; };
 private:
  const int group_to_extract_{0};
};

} // namespace bart::instrumentation::converter::system

#endif //BART_SRC_INSTRUMENTATION_CONVERTER_SYSTEM_MOMENT_EXTRACTOR_HPP_
