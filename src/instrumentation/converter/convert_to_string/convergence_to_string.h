#ifndef BART_SRC_INSTRUMENTATION_CONVERTER_CONVERGENCE_TO_STRING_H_
#define BART_SRC_INSTRUMENTATION_CONVERTER_CONVERGENCE_TO_STRING_H_

#include <map>
#include <string>
#include <variant>
#include <vector>

#include "instrumentation/converter/convert_to_string/to_string_converter.h"
#include "convergence/status.hpp"

namespace bart {

namespace instrumentation {

namespace converter {

namespace convert_to_string {

enum class ConvergenceToStringOutputTerm {
  kIterationNum = 0, kIterationMax = 1, kDelta = 2, kIndex = 3
};

class ConvergenceToString :
    public ToStringConverter<convergence::Status, ConvergenceToStringOutputTerm> {
 public:
  using OutputTerm = ConvergenceToStringOutputTerm;
  using OutputTermToStringMap = std::map<OutputTerm, std::string>;

  ConvergenceToString();
  virtual ~ConvergenceToString() = default;
  std::string Convert(const convergence::Status& to_convert) const override;

  std::string null_character() const { return null_character_; };

 private:
  std::string null_character_{"∅"};
  static bool is_registered_;
};



} // namespace convert_to_string

} // namespace converter

} // namespace instrumentation

} // namespace bart

#endif //BART_SRC_INSTRUMENTATION_CONVERTER_CONVERGENCE_TO_STRING_H_
