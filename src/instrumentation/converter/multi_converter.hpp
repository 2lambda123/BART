#ifndef BART_SRC_INSTRUMENTATION_CONVERTER_MULTI_CONVERTER_HPP_
#define BART_SRC_INSTRUMENTATION_CONVERTER_MULTI_CONVERTER_HPP_

#include <memory>

#include "instrumentation/converter/converter_i.h"
#include "utility/has_dependencies.h"

namespace bart::instrumentation::converter {

template <typename InputType, typename IntermediateType, typename OutputType>
class MultiConverter
    : public ConverterI<InputType, OutputType>,
      public utility::HasDependencies {
 public:
  using FirstStageConverter = instrumentation::converter::ConverterI<InputType, IntermediateType>;
  using FirstStageConverterPtr = std::unique_ptr<FirstStageConverter>;
  using SecondStageConverter = instrumentation::converter::ConverterI<IntermediateType, OutputType>;
  using SecondStageConverterPtr = std::unique_ptr<SecondStageConverter>;
  MultiConverter(FirstStageConverterPtr first_stage_converter_ptr,
                 SecondStageConverterPtr second_stage_converter_ptr)
      : first_stage_converter_ptr_(std::move(first_stage_converter_ptr)),
        second_stage_converter_ptr_(std::move(second_stage_converter_ptr)) {
    std::string call_location{"Multiconverter constructor"};
    AssertPointerNotNull(first_stage_converter_ptr_.get(),
                         "first-stage converter",
                         call_location);
    AssertPointerNotNull(second_stage_converter_ptr_.get(),
                         "second-stage converter",
                         call_location); }
  OutputType Convert(const InputType &input) const override {
    return second_stage_converter_ptr_->Convert(
        first_stage_converter_ptr_->Convert(input));
  }

  FirstStageConverter* first_stage_converter_ptr() {
    return first_stage_converter_ptr_.get(); }
  SecondStageConverter* second_stage_converter_ptr() {
    return second_stage_converter_ptr_.get(); }
 private:
  FirstStageConverterPtr first_stage_converter_ptr_;
  SecondStageConverterPtr second_stage_converter_ptr_;
};

template <typename InputType, typename IntermediateType, typename OutputType>
inline std::unique_ptr<ConverterI<InputType, OutputType>>
operator+(
    std::unique_ptr<ConverterI<InputType, IntermediateType>> lhs,
    std::unique_ptr<ConverterI<IntermediateType, OutputType>> rhs) {
  return std::make_unique<MultiConverter<InputType, IntermediateType, OutputType>>(
      std::move(lhs), std::move(rhs));
}


} // namespace bart::instrumentation::converter

#endif //BART_SRC_INSTRUMENTATION_CONVERTER_MULTI_CONVERTER_HPP_
