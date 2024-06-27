/*
 * any_unittests.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/any.h"
#include "gtest/gtest.h"
#include "test/gtest.h"

namespace ave {
namespace base {

namespace {
class Object {
 public:
  Object(int i) : value_(i) {}
  virtual ~Object() = default;
  int GetValue() { return value_; }

 private:
  int value_;
};
}  // namespace

TEST(AnyTest, CtorTest) {
  Any a;
}

TEST(AnyTest, SetGetTest) {
  Any a;
  int k = 1;
  a.Set<Object>(k);

  auto* obj = a.Get<Object>();
  EXPECT_EQ(obj->GetValue(), k);
}

TEST(AnyTest, GetNullTest) {
  Any a;
  auto* obj = a.Get<Object>();
  EXPECT_EQ(obj, nullptr);
}

TEST(AnyTest, EmptyTest) {
  Any a;
  EXPECT_TRUE(a.empty());
  int k = 1;
  a.Set<Object>(k);
  EXPECT_FALSE(a.empty());
}

}  // namespace base
}  // namespace ave
