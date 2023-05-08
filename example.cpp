// main.cpp

#include <high_level/hl1.h>
#include <high_level/hl2.h>
#include <high_level/hl3.h>

int main()
{
  HL1 hl1(1, 'foo');
  HL2 hl2(2, 'bar');
  HL3 hl3(42, 'baz', 2.3);

  // do stuff...

  return 0;
}

// hl1.h

#include <mid_level/ml1.h>
#include <mid_level/ml2.h>

class HL1
{
  public:
    HL1() = default;

    void doSomething(const ML1& ml1);
    ML2 getValue() const;

};

// ml1.h

#include <low_level/ll1.h>
#include <low_level/ll2.h>

class ML1
{
  public:
    ML1() = default;

    void doSomething(const LL1& ml1);
    LL2 getValue() const;

};


//////// REALISTIC EXAMPLE //////////

// message_data.eos
// TODO: write simple eos message with an int...
struct MessageData
{
  val: int64
}

// solver.h
#include <value_modifier.h>

class Solver
{
  public:
    explicit Solver(const int clipping_limit) : clipping_limit_(clipping_limit)
    {
      // generate curr_data_;
      // TODO: remove the ctor call to give it curr_data_
      value_modifier_ = ValueModifier(curr_data_);
    }

    /**
     * @brief Callback function called by another component.
     *
     * @param msg The message containing the updated data.
     */
    void updateDataCb(const MessageData& msg)
    {
      curr_data_ = msg;
      value_modifier_.update(msg);
    }

    int solve()
    {
      // applies a clipping function to limit the value to 30
      const int val = value_modifier_.generateVal();
      return min(clipping_limit_, val);
    }

  private:
    int clipping_limit_{0};
    MessageData curr_data_;
    ValueModifier value_modifier_;
};

// value_modifier.h

class ValueModifier
{
  public:
    ValueModifier() = default;

    void update(const MessageData& msg)
    {
      curr_data_ = msg;
    }

    int generateVal()
    {
      return curr_data_.get_val() * curr_data_.get_val();
    }

  private:
    MessageData curr_data_;
};


// Problem: how do I test Solver? ValueModifier?
// Possible answer: could leverage what I know about ValueModifier and expect solve() to return val()**3
// - Unit test ValueModifier independently
// - Unit test Solver that depends on ValueModifier
//
// What if I change the underlying behavior of my ValueModifier to instead calculate the fibbonacci?
// - Update ValueModifier tests
// - Update Solver tests.
//
// Imagine these are at the bottom level of my call tree
//

// Solution:


class Solver
{
  public:
    explicit Solver(const int clipping_limit, std::unique_ptr<IValueModifier> value_modifier_ptr) : clipping_limit_(clipping_limit),
      value_modifier_ptr_(std::move(value_modifier_ptr))
    {
      // generate curr_data_;
    }

    /**
     * @brief Callback function called by another component.
     *
     * @param msg The message containing the updated data.
     */
    void updateDataCb(const MessageData& msg)
    {
      curr_data_ = msg;
      value_modifier_ptr_->update(msg);
    }

    int solve()
    {
      const int val = value_modifier_ptr_->generateVal();
      if (val > 10) {
        // TODO: apply some behavior
      } else {
        // TODO: some other behavior
      }
    }

  private:
    MessageData curr_data_;
    std::unique_ptr<ValueModifier> value_modifier_ptr_;
};

class IValueModifier
{
  public:
    /**
     * @brief Default virtual destructor required for inheritance
     */
    virtual ~IValueModifier() = default;

    virtual void updateDataCb(const MessageData& msg) = 0;
    virtual int generateVal() = 0;
};

class SquareValueModifier : public ValueModifier
{
  public:
    void updateDataCb(const MessageData& msg) override
    {
      curr_data_ = msg;
    }

    int generateVal() override
    {
      return curr_data_.get_val() * curr_data_.get_val();
    }

  private:
    MessageData curr_data_;
};


// Show how I can now mock this out with e.g., gmock
//
// What if I now want to calculate the fibbonacci instad of square?
// - simply create a different implementation of IValueModifier which performs the fibbonacci
//
// How can I create my concrete object?
//
// TODO: incorporate rules of Dependency Inversion Principle (avoiding concrete dependencies at any cost)

// solver_test.cpp

#include <solver.h>
#include <value_modifier_interface.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

class MockValueModifier : public IValueModifier
{
  MOCK_METHOD1(update, void(const MessageData& msg));
  MOCK_METHOD0(generateVal, int());
};

TEST(SolverTest, updateDataCbCallsMockValueGenUpdate)
{
  // arrange
  std::unique_ptr<MockValueModifier> mock_gen_uptr;
  MockValueModifier* mock_gen_ptr = mock_gen_uptr.get();

  const int clipping_limit = 30;
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

  const int clipping_limit = 30;
  Solver solver(clipping_limit, std::move(mock_gen_uptr));

  // act & assert
  // define the mocked generateVal() to return 50 when called
  EXPECT_CALL((*mock_gen_ptr, generateVal)).Times(1).WillRepeatedly(Return(clipping_limit + 10));
  MessageData data(42);

  // expect the returned solution to be clipped
  EXPECT_EQ(clipping_limit, solver.solve());
}

// TODO: consolidate above tests into a single test below for demonstration?
TEST(SolverTest, solveWithValueBelowClippingValue)
{
  // arrange
  std::unique_ptr<MockValueModifier> mock_gen_uptr;
  MockValueModifier* mock_gen_ptr = mock_gen_uptr.get();

  const int clipping_limit = 30;
  Solver solver(clipping_limit, std::move(mock_gen_uptr));

  // act & assert
  // define the mocked generateVal() to return 50 when called
  const int returned_val = 0.5 * clipping_limit;
  EXPECT_CALL((*mock_gen_ptr, generateVal)).Times(1).WillRepeatedly(Return(returned_val));
  MessageData data(20);
  EXPECT_EQ(returned_val, solver.solve());
}

// What if I want to easily experiment with differnt value modifiers?
// e.g., SquareValueModifier, LogValueModifier, LinearValueModifier, etc.


/*************************************************************************
 * Factory Pattern
 ************************************************************************/

// value_modifier_factory.h
#include <value_modifier_interface.h>
#include <value_modifier_lib/square_modifier.h>
#include <value_modifier_lib/log_modifier.h>

class ValueModifierFactory
{
  public:
    enum class ModifierType
    {
      SQUARE,
      LOG
    }

    static std::unique_ptr<IValueModifier> makeValueModifier(const ModifierType& mod_type)
    {
      if (mod_type == SQUARE) {
        return std::make_unique<SquareValueModifier>();
      } else if (mod_type == LOG)
      {
        return std::make_unique<LogValueModifier>();
      }
      else {
        throw std::runtime_error("Unknown value modifier type: " + std::to_string(mod_type));
      }
    }
};

/*************************************************************************
 * Main
 ************************************************************************/

#include <value_modifier_factory.h>
#include <value_modifier_interface.h>

int main()
{
  const int clipping_limit = 42;

  std::unique_ptr<IValueModifier> square_modifier_ptr = ValueModifierFactory::makeValueModifier(ValueModifierFactory::ModifierType::SQUARE);
  Solver sq_solver(clipping_limit, ValueModifierFactory::makeValueModifier(std::move(square_modifier_ptr)));

  std::unique_ptr<IValueModifier> log_modifier_ptr = ValueModifierFactory::makeValueModifier(ValueModifierFactory::ModifierType::LOG);
  Solver log_solver(clipping_limit, ValueModifierFactory::makeValueModifier(ValueModifierFactory::ModifierType::LOG));


  sq_solver.updateDataCb()

  return 0;
}
