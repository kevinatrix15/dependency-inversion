// #include <value_modifier.h>

#include <gtest/gtest.h>

class ValueModifier;
struct MessageData;

TEST(ValueModifierTest, modifiedValueBelowClippingLimit) {
  ValueModifier modifier;

  const int in_val = 3;
  modifier.update(MessageData(in_val));

  EXPECT_EQ(in_val * in_val, modifier.generateVal());
}
