#ifndef BART_SRC_INSTRUMENTATION_CONVERTER_DOUBLE_TO_STRING_H_
#define BART_SRC_INSTRUMENTATION_CONVERTER_DOUBLE_TO_STRING_H_

#include <map>
#include <string>
#include <vector>
#include <variant>

#include "instrumentation/converter/to_string_converter.h"

namespace bart {

namespace instrumentation {

namespace converter {

enum DoubleToStringOutputTerm { kValue, };

class DoubleToString :
    public ToStringConverter<double, DoubleToStringOutputTerm> {
 public:
  using OutputTerm = DoubleToStringOutputTerm;
  using OutputTermToStringMap = std::map<OutputTerm, std::string>;

  DoubleToString()
      : ToStringConverter<double, DoubleToStringOutputTerm>(
      "${VALUE}\n", {{kValue, "${VALUE}"}}) {}
  virtual ~DoubleToString() = default;

  std::string Convert(const double &input) const override;

  std::string SetOutputFormat(
      const std::vector<std::variant<OutputTerm, std::string>>);

  DoubleToString& set_precision(const int to_set) {
    precision_ = to_set;
    return *this; }

  int precision() const { return precision_; }

 private:
  int precision_ = 2;
};

} // namespace converter

} // namespace instrumentation

} // namespace bart

#endif //BART_SRC_INSTRUMENTATION_CONVERTER_DOUBLE_TO_STRING_H_
