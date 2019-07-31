#include "framework/framework.h"

#include "iteration/outer/tests/outer_iteration_mock.h"
#include "iteration/initializer/tests/initializer_mock.h"
#include "test_helpers/gmock_wrapper.h"

namespace  {

using namespace bart;

using ::testing::Ref;

class FrameworkTest : public ::testing::Test {
 public:
  using Framework = framework::Framework;
  using Initializer = iteration::initializer::InitializerMock;
  using OuterIterator = iteration::outer::OuterIterationMock;

  std::unique_ptr<Framework> test_framework_;

  // Observation Pointers
  system::System* system_obs_ptr_;
  Initializer* initializer_obs_ptr_;
  OuterIterator* outer_iterator_obs_ptr_;

  void SetUp() override;
};

void FrameworkTest::SetUp() {
  auto system_ptr = std::make_unique<system::System>();
  system_obs_ptr_ = system_ptr.get();
  auto initializer_ptr = std::make_unique<Initializer>();
  initializer_obs_ptr_ = initializer_ptr.get();
  auto outer_iterator_ptr = std::make_unique<OuterIterator>();
  outer_iterator_obs_ptr_ = outer_iterator_ptr.get();

  test_framework_ = std::make_unique<Framework>(
      std::move(system_ptr),
      std::move(initializer_ptr),
      std::move(outer_iterator_ptr)
      );
}

TEST_F(FrameworkTest, Constructor) {
  EXPECT_NE(test_framework_->system(), nullptr);
  EXPECT_NE(test_framework_->initializer_ptr(), nullptr);
  EXPECT_NE(test_framework_->outer_iterator_ptr(), nullptr);
}

TEST_F(FrameworkTest, ConstructorThrows) {
  for (int i = 0; i < 3; ++i) {
    auto system_ptr = (i == 0) ? nullptr :
        std::make_unique<system::System>();
    auto initializer_ptr = (i == 1) ? nullptr :
        std::make_unique<Initializer>();
    auto outer_iterator_ptr = (i == 2) ? nullptr :
        std::make_unique<OuterIterator>();
    EXPECT_ANY_THROW({
                       Framework test_framework(
                           std::move(system_ptr),
                           std::move(initializer_ptr),
                           std::move(outer_iterator_ptr)
                       );
    });
  }
}

TEST_F(FrameworkTest, SolveSystem) {
  auto& system = *system_obs_ptr_;

  EXPECT_CALL(*initializer_obs_ptr_, Initialize(Ref(system)));
  EXPECT_CALL(*outer_iterator_obs_ptr_, IterateToConvergence(Ref(system)));

  test_framework_->SolveSystem();
}



} // namespace