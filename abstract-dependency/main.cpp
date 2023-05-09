#include <cassert>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

/*************************************************************************
 * MessageData
 ************************************************************************/

// TODO: replace with EOSLang struct
struct MessageData
{
  MessageData() = default;

  MessageData(const double val) : val_(val)
  {
  }

  double get_val() const
  {
    return val_;
  }

 private:
  double val_{0};
};

/*************************************************************************
 * ValueModifiers
 ************************************************************************/

// <value_modifiers/value_modifier_interface.h>
class IValueModifier
{
 public:
  /**
   * @brief Default virtual destructor required for inheritance
   */
  virtual ~IValueModifier() = default;

  virtual void update(const MessageData& msg) = 0;
  virtual double generateVal() = 0;
};

// <value_modifiers/square_value_modifier.h>
class SquareValueModifier : public IValueModifier
{
 public:
  void update(const MessageData& msg) override
  {
    curr_data_ = msg;
  }

  double generateVal() override
  {
    return curr_data_.get_val() * curr_data_.get_val();
  }

 private:
  MessageData curr_data_;
};

// <value_modifiers/log_value_modifier.h>
#include <cmath>

class LogValueModifier : public IValueModifier
{
 public:
  void update(const MessageData& msg) override
  {
    curr_data_ = msg;
  }

  double generateVal() override
  {
    return static_cast<double>(std::log(curr_data_.get_val()));
  }

 private:
  MessageData curr_data_;
};

// What if I want to easily experiment with differnt value modifiers?
// e.g., SquareValueModifier, LogValueModifier, LinearValueModifier, etc.

/*************************************************************************
 * Factory Pattern
 ************************************************************************/

// value_modifier_factory.h
// #include <value_modifier_interface.h>
// #include <value_modifier_lib/log_modifier.h>
// #include <value_modifier_lib/square_modifier.h>

class ValueModifierFactory
{
 public:
  enum class ModifierType
  {
    SQUARE,
    LOG
  };

  static std::unique_ptr<IValueModifier> makeValueModifier(const ModifierType& mod_type)
  {
    if (mod_type == ModifierType::SQUARE)
    {
      return std::make_unique<SquareValueModifier>();
    }
    else if (mod_type == ModifierType::LOG)
    {
      return std::make_unique<LogValueModifier>();
    }
    else
    {
      throw std::runtime_error("Unknown value modifier encountered");
    }
  }
};

/*************************************************************************
 * Solver
 ************************************************************************/

// #include <value_modifier/value_modifier_interface.h>

class Solver
{
 public:
  explicit Solver(const double clipping_limit, std::unique_ptr<IValueModifier> value_modifier_ptr)
    : clipping_limit_(clipping_limit), value_modifier_ptr_(std::move(value_modifier_ptr))
  {
    assert(value_modifier_ptr_);
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

  double solve()
  {
    // limit the value to the clipping_limit
    const double val = value_modifier_ptr_->generateVal();
    return std::min(clipping_limit_, val);
  }

 private:
  double clipping_limit_{0};
  MessageData curr_data_;
  std::unique_ptr<IValueModifier> value_modifier_ptr_{nullptr};
};

/*************************************************************************
 * Main
 ************************************************************************/

// #include <value_modifier_factory.h>
// #include <value_modifier_interface.h>

int main()
{
  const double clipping_limit = 42;

  std::unique_ptr<IValueModifier> square_modifier_ptr =
      ValueModifierFactory::makeValueModifier(ValueModifierFactory::ModifierType::SQUARE);
  Solver sq_solver(clipping_limit, std::move(square_modifier_ptr));

  std::unique_ptr<IValueModifier> log_modifier_ptr =
      ValueModifierFactory::makeValueModifier(ValueModifierFactory::ModifierType::LOG);
  Solver log_solver(clipping_limit, std::move(log_modifier_ptr));

  const size_t n = 10;
  std::vector<double> vals(n);
  std::iota(vals.begin(), vals.end(), 0);

  for (const auto v : vals)
  {
    const MessageData data(v);
    sq_solver.updateDataCb(data);
    log_solver.updateDataCb(data);

    const double sq_sln = sq_solver.solve();
    const double log_sln = log_solver.solve();
    std::cout << "Solver w/ clipping_limit: " << std::to_string(clipping_limit)
              << ", given data.value() = " << data.get_val() << std::endl;
    std::cout << ", sq_solver produces: " << std::to_string(sq_sln)
              << ", log_solver produces: " << std::to_string(log_sln) << std::endl;
  }

  return 0;
}

