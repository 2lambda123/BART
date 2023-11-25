#include "framework/framework_helper.hpp"
#include "framework/tests/framework_helper_mock.hpp"
#include "system/tests/system_helper_mock.hpp"

#include "test_helpers/gmock_wrapper.h"

namespace  {

using namespace bart;
using ::testing::NotNull, ::testing::WhenDynamicCastTo;

template <typename DimensionWrapper>
class FrameworkHelperConstructorTests : public ::testing::Test {
 public:
  static constexpr int dim = DimensionWrapper::value;
};

TYPED_TEST_SUITE(FrameworkHelperConstructorTests, bart::testing::AllDimensions);

TYPED_TEST(FrameworkHelperConstructorTests, Constructor) {
  constexpr int dim = this->dim;
  using SystemHelper = const system::SystemHelperMock<dim>;
  auto system_helper_ptr = std::make_shared<SystemHelper>();
  framework::FrameworkHelper<dim> helper(system_helper_ptr);
  EXPECT_THAT(helper.system_helper_ptr(), WhenDynamicCastTo<SystemHelper*>(NotNull()));
}

TYPED_TEST(FrameworkHelperConstructorTests, ConstructorNullDependency) {
  constexpr int dim = this->dim;
  EXPECT_ANY_THROW({
                     framework::FrameworkHelper<dim> helper(nullptr);
                   });

}

TYPED_TEST(FrameworkHelperConstructorTests, SetSubrountineFrameworkHelper) {
  constexpr int dim = this->dim;
  using SystemHelper = const system::SystemHelperMock<dim>;
  auto system_helper_ptr = std::make_shared<SystemHelper>();
  framework::FrameworkHelper<dim> helper(system_helper_ptr);
  EXPECT_THAT(helper.system_helper_ptr(), WhenDynamicCastTo<SystemHelper*>(NotNull()));
  EXPECT_EQ(helper.subroutine_framework_helper_ptr(), nullptr);
  helper.SetSubroutineFrameworkHelper(std::make_unique<framework::FrameworkHelperMock<dim>>());
  EXPECT_NE(helper.subroutine_framework_helper_ptr(), nullptr);
}

} // namespace
