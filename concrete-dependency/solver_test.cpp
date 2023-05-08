// #include <solver.h>

#include <gtest/gtest.h>

TEST(SolverTest, modifiedValueBelowClippingLimit) {

  const int clipping_limit = 42;

  Solver solver(clipping_limit);

  MessageData data(6);
  solver.updateDataCb(data);

  // expect the solved result = val^2
  EXPECT_EQ(data.get_val() * data.get_val(), solver.solve());
}

TEST(SolverTest, modifiedValueAboveClippingLimit) {

  const int clipping_limit = 42;

  Solver solver(clipping_limit);

  MessageData data(7);
  solver.updateDataCb(data);

  // expect the solved result = clipping_limit
  EXPECT_EQ(clipping_limit, solver.solve());
}
