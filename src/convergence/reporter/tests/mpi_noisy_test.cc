#include "convergence/reporter/mpi_noisy.h"

#include <deal.II/base/conditional_ostream.h>
#include <gtest/gtest.h>

#include "convergence/status.h"
#include "test_helpers/gmock_wrapper.h"

namespace  {

using namespace bart;

class ReporterMpiNoisyTest : public ::testing::Test {
 protected:
  std::ostringstream string_stream;
  std::unique_ptr<dealii::ConditionalOStream> pout_ptr;
  void SetUp() override;
};

void ReporterMpiNoisyTest::SetUp() {
  pout_ptr = std::make_unique<dealii::ConditionalOStream>(string_stream, true);
}

/* Verifies that the constructor takes ownership of the conditional ostream
 * pointer */
TEST_F(ReporterMpiNoisyTest, Constructor) {
  bart::convergence::reporter::MpiNoisy reporter(std::move(pout_ptr));
  EXPECT_EQ(pout_ptr, nullptr);
}

/* Verifies that the reporter sends the string to the ostream */
TEST_F(ReporterMpiNoisyTest, StringReport) {
  bart::convergence::reporter::MpiNoisy reporter(std::move(pout_ptr));
  std::string to_report = "report me";
  reporter.Report(to_report);
  EXPECT_EQ(string_stream.str(), to_report);
}

TEST_F(ReporterMpiNoisyTest, ConvergenceReport) {
  bart::convergence::reporter::MpiNoisy reporter(std::move(pout_ptr));
  bart::convergence::Status to_report;
  to_report.iteration_number = 12;
  to_report.max_iterations = 100;
  to_report.delta = 1e-4;
  to_report.failed_index = 4;
  reporter.Report(to_report);
  EXPECT_EQ(string_stream.str(),
      "Iteration: 12/100\tdelta: 0.0001\tidx: 4\n");
}

TEST_F(ReporterMpiNoisyTest, ConvergenceReportNullValues) {
  bart::convergence::reporter::MpiNoisy reporter(std::move(pout_ptr));
  bart::convergence::Status null_status;
  null_status.iteration_number = 12;
  null_status.max_iterations = 100;
  null_status.failed_index = 4;

  reporter.Report(null_status);
  EXPECT_EQ(string_stream.str(),
            "Iteration: 12/100\tdelta: N/A\tidx: 4\n");

  string_stream.str("");

  null_status.failed_index = std::nullopt;
  null_status.delta = 0.0001;

  reporter.Report(null_status);
  EXPECT_EQ(string_stream.str(),
            "Iteration: 12/100\tdelta: 0.0001\tidx: N/A\n");

  string_stream.str("");

  null_status.failed_index = std::nullopt;
  null_status.delta = std::nullopt;

  reporter.Report(null_status);
  EXPECT_EQ(string_stream.str(),
            "Iteration: 12/100\tdelta: N/A\tidx: N/A\n");
}

} // namespace