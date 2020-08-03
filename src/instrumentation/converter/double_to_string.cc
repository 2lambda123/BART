#include "instrumentation/converter/double_to_string.h"

#include <iomanip>
#include <sstream>

namespace bart {

namespace instrumentation {

namespace converter {

DoubleToString::DoubleToString()
    : ToStringConverter<double, DoubleToStringOutputTerm>(
    "${VALUE}\n", {{kValue, "${VALUE}"}}) {}

std::string DoubleToString::Convert(const double &input) const {
  std::string return_string{output_format_};
  std::string value_string{output_term_to_string_map_.at(OutputTerm::kValue)};

  if (auto index = return_string.find(value_string);
      index != std::string::npos) {
    std::ostringstream value_stream;
    value_stream << std::fixed << std::setprecision(precision_) << input;
    return_string.replace(index, value_string.size(), value_stream.str());
  }
  return return_string;
}

} // namespace converter

} // namespace instrumentation

} // namespace bart
