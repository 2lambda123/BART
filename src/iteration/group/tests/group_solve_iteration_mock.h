#ifndef BART_SRC_ITERATION_GROUP_TESTS_GROUP_SOLVE_ITERATION_MOCK_H_
#define BART_SRC_ITERATION_GROUP_TESTS_GROUP_SOLVE_ITERATION_MOCK_H_

#include "iteration/group/group_solve_iteration_i.h"

#include "test_helpers/gmock_wrapper.h"

namespace bart {

namespace iteration {

namespace group {

class GroupSolveIterationMock : public GroupSolveIterationI {
 public:
  MOCK_METHOD1(Iterate, void(system::System &system));
};

} // namespace group

} // namespace iteration

} // namespace bart

#endif //BART_SRC_ITERATION_GROUP_TESTS_GROUP_SOLVE_ITERATION_MOCK_H_
