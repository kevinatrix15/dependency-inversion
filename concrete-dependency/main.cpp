#include <iostream>
#include <memory>
#include <numeric>
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
 * ValueModifier
 ************************************************************************/

class ValueModifier
{
 public:
  void update(const MessageData& msg)
  {
    curr_data_ = msg;
  }

  double generateVal()
  {
    return curr_data_.get_val() * curr_data_.get_val();
  }

 private:
  MessageData curr_data_;
};

/*************************************************************************
 * Solver
 ************************************************************************/

// #include <value_modifier.h>

class Solver
{
 public:
  explicit Solver(const double clipping_limit) : clipping_limit_(clipping_limit), curr_data_(), value_modifier_()
  {
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

  double solve()
  {
    // limit the value to the clipping_limit
    const double val = value_modifier_.generateVal();
    return std::min(clipping_limit_, val);
  }

 private:
  double clipping_limit_{0};
  MessageData curr_data_;
  ValueModifier value_modifier_;
};

/*************************************************************************
 * Main
 ************************************************************************/

// #include <value_modifier_factory.h>
// #include <value_modifier_interface.h>

int main()
{
  const double clipping_limit = 42;

  Solver solver(clipping_limit);

  const size_t n = 10;
  std::vector<double> vals(n);
  std::iota(vals.begin(), vals.end(), 0);

  for (const auto v : vals)
  {
    const MessageData data(v);
    solver.updateDataCb(data);

    const double sln = solver.solve();
    std::cout << "Solver w/ clipping_limit: " << std::to_string(clipping_limit)
              << ", given data.value() = " << data.get_val() << ", produces: " << std::to_string(sln) << std::endl;
  }

  return 0;
}
