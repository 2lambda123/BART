//#include "../bart_builder.h"
#include "../problem_definition.h"

#include "gtest/gtest.h"
#include "../../test_helpers/bart_test_helper.h"

class BARTBuilderTest : public ::testing::Test {
 protected:
  void SetUp ();

  // // FE builder test
  // void SetUpFEBuilderTest ();

  // template <int dim>
  // void FEBuilderTest ();

  dealii::ParameterHandler prm;
};

void BARTBuilderTest::SetUp () {
  // Note that setup for different tests
  // had better be separated for clearness.

  //  SetUpFEBuilderTest();
}

// void BARTBuilderTest::SetUpFEBuilderTest () {
//   prm.declare_entry("finite element polynomial degree", "1",
//                     dealii::Patterns::Integer(), "");
//   prm.declare_entry("do nda", "true",
//                     dealii::Patterns::Bool(), "");
//   prm.declare_entry("transport model", "dummy",
//                     dealii::Patterns::Anything(), "");
//   prm.declare_entry("ho spatial discretization", "",
//                     dealii::Patterns::Anything(), "");
//   prm.declare_entry("nda spatial discretization", "",
//                     dealii::Patterns::Anything(), "");
// }

// template <int dim>
// void BARTBuilderTest::FEBuilderTest () {
//   // set values to parameters
//   prm.set ("ho spatial discretization", "cfem");
//   prm.set ("nda spatial discretization", "cfem");
//   std::unordered_map<std::string, dealii::FiniteElement<dim, dim>*> fe_ptrs;
//   bbuilders::BuildFESpaces (prm, fe_ptrs);

//   // testing for FE names
//   EXPECT_EQ (fe_ptrs["dummy"]->get_name(),
//       "FE_Q<"+dealii::Utilities::int_to_string(dim)+">(1)");
//   EXPECT_EQ (fe_ptrs["nda"]->get_name(),
//       "FE_Q<"+dealii::Utilities::int_to_string(dim)+">(1)");

//   // changing FE types
//   prm.set ("ho spatial discretization", "dfem");
//   prm.set ("nda spatial discretization", "dfem");
//   fe_ptrs.clear ();
//   bbuilders::BuildFESpaces (prm, fe_ptrs);
//   EXPECT_EQ (fe_ptrs["dummy"]->get_name(),
//       "FE_DGQ<"+dealii::Utilities::int_to_string(dim)+">(1)");
//   EXPECT_EQ (fe_ptrs["nda"]->get_name(),
//       "FE_DGQ<"+dealii::Utilities::int_to_string(dim)+">(1)");

//   // changing NDA FE type
//   prm.set ("nda spatial discretization", "cmfd");
//   fe_ptrs.clear ();
//   bbuilders::BuildFESpaces (prm, fe_ptrs);
//   EXPECT_EQ (fe_ptrs["nda"]->get_name(),
//       "FE_DGQ<"+dealii::Utilities::int_to_string(dim)+">(0)");

//   // changing NDA FE type
//   prm.set ("nda spatial discretization", "rtk");
//   fe_ptrs.clear ();
//   bbuilders::BuildFESpaces (prm, fe_ptrs);
//   EXPECT_EQ (fe_ptrs["nda"]->get_name(),
//       "FE_RaviartThomas<"+dealii::Utilities::int_to_string(dim)+">(1)");
// }

// TEST_F (BARTBuilderTest, FEBuilder2DTest) {
//   FEBuilderTest<2> ();
// }

// TEST_F (BARTBuilderTest, FEBuilder3DTest) {
//   FEBuilderTest<3> ();
// }
