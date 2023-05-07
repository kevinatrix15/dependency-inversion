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

// solver.h
#include <helper.h>

class Solver
{
  public:
    Solver()
    {
      // generate curr_data_;
      // TODO: remove the ctor call to give it curr_data_
      helper_ = Helper(curr_data_);
    }

    /**
     * @brief Callback function called by another component.
     *
     * @param msg The message containing the updated data.
     */
    void updateDataCb(const MessageData& msg)
    {
      curr_data_ = msg;
      helper_.update(msg);
    }

    void solve()
    {
      // TODO: replace this with some logic owned by Solver
      curr_data_.val() * helper_.processVal();
    }

  private:
    MessageData curr_data_;
    Helper helper_;
};

// helper.h

class Helper
{
  public:
    Helper() = default;

    void update(const MessageData& msg)
    {
      curr_data_ = msg;
    }

    int processVal()
    {
      return curr_data_.val() * curr_data_.val();
    }

  private:
    MessageData curr_data_;
};


// Problem: how do I test Solver? Helper?
// Possible answer: could leverage what I know about Helper and expect solve() to return val()**3
// - Unit test Helper independently
// - Unit test Solver that depends on Helper
//
// What if I change the underlying behavior of my Helper to instead calculate the fibbonacci?
// - Update Helper tests
// - Update Solver tests.
//
// Imagine these are at the bottom level of my call tree
//

// Solution:


class Solver
{
  public:
    Solver(std::unique_ptr<IHelper> helper_ptr) : helper_ptr_(std::move(helper_ptr))
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
      helper_ptr_->update(msg);
    }

    void solve()
    {
      curr_data_.val() * helper_ptr_->processVal();
    }

  private:
    MessageData curr_data_;
    std::unique_ptr<Helper> helper_ptr_;
};

class IHelper
{
  public:
    /**
     * @brief Default virtual destructor with inheritance
     */
    virtual ~IHelper() = default;

    virtual void updateDataCb(const MessageData& msg) = 0;
    virtual int processVal() = 0;
};

// TODO: rename from Helper to something more meaningful
class SquareHelper : public Helper
{
  public:
    void updateDataCb(const MessageData& msg) override
    {

    }

    virtual int processVal() = 0;

  private:

};


// Show how I can now mock this out with e.g., gmock
//
// What if I now want to calculate the fibbonacci instad of square?
// - simply create a different implementation of IHelper which performs the fibbonacci
//
// How can I create my concrete object?
//
//
// TODO: incorporate rules of Dependency Inversion Principle (avoiding concrete dependencies at any cost)
