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
 * Abstract Factory Pattern
 ************************************************************************/

// value_modifier_factory_interface.h
// #include <value_modifier_interface.h>

class IValueModifierFactory
{
 public:
  enum class ModifierType
  {
    SQUARE,
    LOG
  };

  virtual ~IValueModifierFactory() = default;

  virtual std::unique_ptr<IValueModifier> makeValueModifier(const ModifierType& mod_type) = 0;
};

// value_modifier_factory.h

// #include <value_modifier_lib/log_modifier.h>
// #include <value_modifier_lib/square_modifier.h>

class ValueModifierFactory : public IValueModifierFactory
{
 public:
  std::unique_ptr<IValueModifier> makeValueModifier(const ModifierType& mod_type) override
  {
    switch (mod_type)
    {
      case ModifierType::SQUARE:
        return std::make_unique<SquareValueModifier>();
        break;
      case ModifierType::LOG:
        return std::make_unique<LogValueModifier>();
        break;
      default:
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
 * Applications
 ************************************************************************/

// #include <value_modifier_factory_interface.h>
// #include <value_modifier_interface.h>

std::unique_ptr<IValueModifier> createValueModifier(IValueModifierFactory& value_modifier_factory,
                                                    IValueModifierFactory::ModifierType mod_type)
{
  return value_modifier_factory.makeValueModifier(mod_type);
}

/**
 * @brief An application that creates its own value modifiers (given e.g., a class of modifier types)
 */
void ComplexApplication(IValueModifierFactory& value_modifier_factory,
                        IValueModifierFactory::ModifierType mod_type,
                        const double clipping_limit)
{
  std::unique_ptr<IValueModifier> val_modifier_ptr = createValueModifier(value_modifier_factory, mod_type);
  Solver solver(clipping_limit, std::move(val_modifier_ptr));

  const size_t n = 10;
  std::vector<double> vals(n);
  std::iota(vals.begin(), vals.end(), 0);

  std::cout << "Solver w/ clipping_limit: " << std::to_string(clipping_limit) << std::endl;
  for (const auto v : vals)
  {
    const MessageData data(v);
    solver.updateDataCb(data);

    const double sln = solver.solve();
    std::cout << "input: = " << data.get_val() << ", output: " << std::to_string(sln) << std::endl;
  }
}

// #include <solver/solver.h>
// #include <value_modifier_interface.h>

/**
 * @brief An application that creates its own value modifiers (given e.g., a class of modifier types)
 */
void SimpleApplication(std::unique_ptr<IValueModifier> value_modifier_ptr, const double clipping_limit)
{
  Solver solver(clipping_limit, std::move(value_modifier_ptr));

  const size_t n = 10;
  std::vector<double> vals(n);
  std::iota(vals.begin(), vals.end(), 0);

  std::cout << "Solver w/ clipping_limit: " << std::to_string(clipping_limit) << std::endl;
  for (const auto v : vals)
  {
    const MessageData data(v);
    solver.updateDataCb(data);

    const double sln = solver.solve();
    std::cout << "input: = " << data.get_val() << ", output: " << std::to_string(sln) << std::endl;
  }
}

/*************************************************************************
 * Main
 ************************************************************************/

// #include <value_modifier_factory.h>

int main()
{
  const double clipping_limit = 42;

  ValueModifierFactory factory;

  std::cout << "*****Running Application() for Square modifier*****" << std::endl;
  std::unique_ptr<IValueModifier> value_modifier =
      factory.makeValueModifier(IValueModifierFactory::ModifierType::SQUARE);
  SimpleApplication(std::move(value_modifier), clipping_limit);

  std::cout << "*****Running Application() for Log modifier*****" << std::endl;
  value_modifier = factory.makeValueModifier(IValueModifierFactory::ModifierType::LOG);
  SimpleApplication(std::move(value_modifier), clipping_limit);

#if 0
  std::cout << "*****Running Application() for Square modifier*****" << std::endl;
  ComplexApplication(factory, IValueModifierFactory::ModifierType::SQUARE, clipping_limit);

  std::cout << "*****Running Application() for Log modifier*****" << std::endl;
  ComplexApplication(factory, IValueModifierFactory::ModifierType::LOG, clipping_limit);
#endif

  return 0;
}

