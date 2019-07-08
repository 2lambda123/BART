#ifndef BART_SRC_CALCULATOR_CELL_TESTS_TOTAL_AGGREGATED_FISSION_SOURCE_MOCK_H_
#define BART_SRC_CALCULATOR_CELL_TESTS_TOTAL_AGGREGATED_FISSION_SOURCE_MOCK_H_

#include "calculator/cell/total_aggregated_fission_source_i.h"
#include "test_helpers/gmock_wrapper.h"

namespace bart {

namespace system {
namespace moments {
class SphericalHarmonicI;
} // namespace moments
} // namespace system

namespace calculator {

namespace cell {

class TotalAggregatedFissionSourceMock : public TotalAggregatedFissionSourceI {
  MOCK_CONST_METHOD1(AggreatedFissionSource, double(
      system::moments::SphericalHarmonicI* system_moments_ptr));
};

} // namespace cell

} // namespace calculator

} // namespace bart

#endif // BART_SRC_CALCULATOR_CELL_TESTS_TOTAL_AGGREGATED_FISSION_SOURCE_MOCK_H_