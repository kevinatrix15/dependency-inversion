// solver_test.cpp

// #include <solver.h>
// #include <value_modifier_interface.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

/*************************************************************************
 * Mocks
 ************************************************************************/

class MockValueModifier : public IValueModifier
{
  MOCK_METHOD1(update, void(const MessageData& msg));
  MOCK_METHOD0(generateVal, double());
};

/*************************************************************************
 * Unit Tests
 ************************************************************************/

TEST(SolverTest, updateDataCbCallsMockValueGenUpdate)
{
  // arrange
  std::unique_ptr<MockValueModifier> mock_gen_uptr;
  MockValueModifier* mock_gen_ptr = mock_gen_uptr.get();

  const double clipping_limit = 30.0;
  Solver solver(clipping_limit, std::move(mock_gen_uptr));

  // act & assert
  EXPECT_CALL((*mock_gen_ptr, update(_))).Times(1);
  MessageData data(42);
  solver.updateDataCb(data);
}

TEST(SolverTest, solveWithValueAboveClippingValue)
{
  // arrange
  std::unique_ptr<MockValueModifier> mock_gen_uptr;
  MockValueModifier* mock_gen_ptr = mock_gen_uptr.get();

  const double clipping_limit = 30.0;
  Solver solver(clipping_limit, std::move(mock_gen_uptr));

  // act & assert
  // define the mocked generateVal() to return 40 when called
  const int returned_val = clipping_limit + 10.0;

  EXPECT_CALL((*mock_gen_ptr, update(_))).Times(1);
  EXPECT_CALL((*mock_gen_ptr, generateVal)).Times(1).WillRepeatedly(Return(returned_val));

  MessageData data(42.0);
  solver.updateDataCb(data);
  const double solution = solver.solve();

  // expect the returned solution to be clipped
  EXPECT_EQ(clipping_limit, solution);
}

TEST(SolverTest, solveWithValueBelowClippingValue)
{
  // arrange
  std::unique_ptr<MockValueModifier> mock_gen_uptr;
  MockValueModifier* mock_gen_ptr = mock_gen_uptr.get();

  const double clipping_limit = 30;
  Solver solver(clipping_limit, std::move(mock_gen_uptr));

  // act & assert
  // define the mocked generateVal() to return 15 when called
  const double returned_val = 0.5 * clipping_limit;

  EXPECT_CALL((*mock_gen_ptr, update(_))).Times(1);
  EXPECT_CALL((*mock_gen_ptr, generateVal)).Times(1).WillRepeatedly(Return(returned_val));

  MessageData data(20.0);
  solver.updateDataCb(data);
  const double solution = solver.solve();

  // expect the returned solution not to be clipped
  EXPECT_EQ(returned_val, solution);
}

