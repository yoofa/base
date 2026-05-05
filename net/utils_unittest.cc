/*
 * utils_unittest.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/net/utils.h"

#include <gtest/gtest.h>

namespace ave {
namespace base {
namespace net {
namespace {

TEST(NetUtilsTest, MakeUserAgent) {
  std::string ua = MakeUserAgent();
  EXPECT_TRUE(ua.find("ave/1.2") != std::string::npos);
  EXPECT_TRUE(ua.find("Linux;Android") != std::string::npos);
}

TEST(NetUtilsTest, UriDebugString) {
  std::string uri = "http://example.com/path?query=1";

  // Incognito
  EXPECT_EQ(UriDebugString(uri, true), "<URI suppressed>");

  // Normal
  EXPECT_EQ(UriDebugString(uri, false), "http://<suppressed>");

  // No scheme
  std::string no_scheme = "example.com/path";
  EXPECT_EQ(UriDebugString(no_scheme, false), "<no-scheme URI suppressed>");

  // Special characters in scheme
  std::string special = "urn:isbn:123";
  EXPECT_EQ(UriDebugString(special, false), "urn://<suppressed>");
}

}  // namespace
}  // namespace net
}  // namespace base
}  // namespace ave
