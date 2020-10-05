#ifndef BART_SRC_INSTRUMENTATION_OUTSTREAM_TO_CONDITIONAL_OSTREAM_H_
#define BART_SRC_INSTRUMENTATION_OUTSTREAM_TO_CONDITIONAL_OSTREAM_H_

#include <deal.II/base/conditional_ostream.h>

#include "instrumentation/outstream/outstream_i.h"
#include "instrumentation/outstream/factory.h"

#include "utility/named_type.h"

namespace bart {

namespace instrumentation {

namespace outstream {

class ToConditionalOstream : public OutstreamI<std::string> {
 public:
  using ConditionalOstreamType = dealii::ConditionalOStream;
  using ConditionalOstreamPtrType = std::unique_ptr<ConditionalOstreamType>;
  ToConditionalOstream(ConditionalOstreamPtrType conditional_ostream_ptr)
      : conditional_ostream_ptr_(std::move(conditional_ostream_ptr)) {}
  ToConditionalOstream& Output(const std::string& to_output) override {
    *conditional_ostream_ptr_ << to_output;
    return *this;
  };

  ConditionalOstreamType* conditional_ostream_ptr() const {
    return conditional_ostream_ptr_.get(); }
 private:
  ConditionalOstreamPtrType conditional_ostream_ptr_ = nullptr;
  inline static bool is_registered_ =
      OutstreamIFactory<std::string, ConditionalOstreamPtrType>::get()
      .RegisterConstructor(OutstreamName::kToConditionalOstream,
          [](ConditionalOstreamPtrType conditional_ostream_ptr)
              -> std::unique_ptr<OutstreamI<std::string>> {
            return std::make_unique<ToConditionalOstream>(
                std::move(conditional_ostream_ptr));
      });
};

} // namespace outstream

} // namespace instrumentation

} // namespace bart

#endif //BART_SRC_INSTRUMENTATION_OUTSTREAM_TO_CONDITIONAL_OSTREAM_H_
