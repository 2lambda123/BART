#include "convergence/final_flux.h"

#include <memory>
#include <optional>

#include "convergence/status.h"
#include "convergence/flux/tests/multi_checker_mock.h"
#include "data/system_fluxes.h"
#include "test_helpers/test_helper_functions.h"
#include "test_helpers/gmock_wrapper.h"

using ::testing::_;

class ConvergenceFinalFluxTest : public ::testing::Test {
 protected:
  std::unique_ptr<bart::convergence::flux::MultiCheckerI> multi_checker_ptr;
  std::unique_ptr<bart::convergence::flux::MultiCheckerMock> mock_multi_checker;
  std::shared_ptr<bart::data::SystemFluxes> system_fluxes;
  void SetUp();
  void MockToPointer();
};

void ConvergenceFinalFluxTest::SetUp() {
  mock_multi_checker =
      std::make_unique<bart::convergence::flux::MultiCheckerMock>();
  system_fluxes = std::make_shared<bart::data::SystemFluxes>();
}

void ConvergenceFinalFluxTest::MockToPointer() {
  multi_checker_ptr = std::move(mock_multi_checker);
}

TEST_F(ConvergenceFinalFluxTest, DefaultStatus) {
  bart::convergence::FinalFlux flux_tester(std::move(mock_multi_checker),
                                           system_fluxes);
  bart::convergence::Status status;
  status = flux_tester.convergence_status();
  ASSERT_EQ(status.iteration_number, 0);
  ASSERT_EQ(status.max_iterations, 1);
  ASSERT_FALSE(status.is_complete);
  ASSERT_FALSE(status.failed_index.has_value());
  ASSERT_FALSE(status.delta.has_value());
}

// TEST_F(ConvergenceFinalFluxTest, BadMaxIterations) {
//   EXPECT_ANY_THROW(flux_tester.SetMaxIterations(0));
//   EXPECT_ANY_THROW(flux_tester.SetMaxIterations(-1));
// }

// TEST_F(ConvergenceFinalFluxTest, GiveMultiChecker) {
//   bart::convergence::FinalFlux flux_tester(mock_multi_checker,
//                                            system_fluxes);
//   EXPECT_EQ(multi_checker_ptr, nullptr);
// }
  

