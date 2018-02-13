#ifdef TEST
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <unistd.h>
#include <cstdlib>

#include "test_helpers/bart_test_helper.h"

#endif

//#include "aqdata/aq_base.h"

#ifdef TEST
int main(int argc, char* argv[]) {
  // Parse optional arguments
  int c;
  while ((c = getopt (argc, argv, "r")) != -1)
    switch(c) {
      case 'r':
        btest::GlobalBartTestHelper().ReInit(true, "test_data/");
    }
  
  // // Testing
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
#else
int main() {
  return 0;
#endif
}
