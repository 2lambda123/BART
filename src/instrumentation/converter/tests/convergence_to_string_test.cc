#include "instrumentation/converter/convergence_to_string.h"

#include <iomanip>
#include <sstream>

#include "convergence/status.h"
#include "test_helpers/test_helper_functions.h"
#include "test_helpers/gmock_wrapper.h"

namespace  {

using namespace bart;

class InstrumentationConverterConvergenceToStringTest : public ::testing::Test {
 public:
  using ConverterType = instrumentation::converter::ConvergenceToString;
  convergence::Status test_status;
  std::string GetExpectedOutput(convergence::Status&, ConverterType&,
                                std::string) const;
  void SetUp() override;
};

void InstrumentationConverterConvergenceToStringTest::SetUp() {
  test_status.is_complete = false;
  test_status.iteration_number = test_helpers::RandomDouble(0, 1000);
  test_status.max_iterations = test_helpers::RandomDouble(1000, 10000);
  test_status.delta = test_helpers::RandomDouble(1e-10, 1e-6);
  test_status.failed_index = test_helpers::RandomDouble(0, 10);
}

std::string InstrumentationConverterConvergenceToStringTest::GetExpectedOutput(
    convergence::Status& status, ConverterType& converter,
    std::string overload_format = "") const {
  using OutputTerm = ConverterType::OutputTerm;
  const auto output_term_to_string_map = converter.output_term_to_string_map();
  std::string output = overload_format;
  if (overload_format == "") {
    output = converter.output_format();
  };

  // Iteration number
  std::string iteration_num_string = output_term_to_string_map.at(OutputTerm::kIterationNum);
  auto iteration_num_index = output.find(iteration_num_string);
  if (iteration_num_index != std::string::npos) {
    output.replace(iteration_num_index,
                   iteration_num_string.size(),
                   std::to_string(status.iteration_number));
  }
  // Max iterations
  std::string iteration_max_string = output_term_to_string_map.at(OutputTerm::kIterationMax);
  auto iteration_max_index = output.find(iteration_max_string);
  if (iteration_max_index != std::string::npos) {
    output.replace(iteration_max_index,
        iteration_max_string.size(),
        std::to_string(status.max_iterations));
  }
  // Delta
  std::ostringstream delta_stream;
  delta_stream << status.delta.value();
  std::string delta_string = output_term_to_string_map.at(OutputTerm::kDelta);
  auto delta_index = output.find(delta_string);
  if (delta_index != std::string::npos) {
    output.replace(delta_index,
                   delta_string.size(),
                   delta_stream.str());
  }
  // Index
  std::string index_string = output_term_to_string_map.at(OutputTerm::kIndex);
  auto index_index = output.find(index_string);
  if (index_index != std::string::npos) {
    output.replace(index_index,
                   index_string.size(),
                   std::to_string(status.failed_index.value()));
  }

  return output;
}

TEST_F(InstrumentationConverterConvergenceToStringTest, Constructor) {
  using ConverterType = instrumentation::converter::ConvergenceToString;
  EXPECT_NO_THROW({
    ConverterType test_converter;
  });
}

TEST_F(InstrumentationConverterConvergenceToStringTest, DefaultString) {
  ConverterType test_converter;
  EXPECT_EQ(test_converter.Convert(test_status),
            GetExpectedOutput(test_status, test_converter));
}

TEST_F(InstrumentationConverterConvergenceToStringTest, ChangeString) {
  ConverterType test_converter;
  using OutputTerm = ConverterType::OutputTerm;
  const auto output_term_to_string_map = test_converter.output_term_to_string_map();

  std::string new_format = "New format: Iterations = " +
      output_term_to_string_map.at(OutputTerm::kIterationNum)
      + " end\n";
  auto returned_format = test_converter.SetOutputFormat({"New format: Iterations = ",
                                                         OutputTerm::kIterationNum,
                                                         " end\n"});
  EXPECT_EQ(new_format, returned_format);
  EXPECT_EQ(test_converter.Convert(test_status),
            GetExpectedOutput(test_status, test_converter, new_format));
}

} // namespace
