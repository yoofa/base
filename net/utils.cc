/*
 * utils.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "utils.h"

#include <string>

namespace ave {
namespace net {

std::string MakeUserAgent() {
  std::string ua;
  ua.append("ave/1.2 (Linux;Android ");

  ua.append(")");
  return ua;
}

std::string UriDebugString(const std::string& uri, bool incognito) {
  if (incognito) {
    return "<URI suppressed>";
  }

  // find scheme
  for (size_t i = 0; i < uri.size(); i++) {
    if (!isascii(uri[i])) {
      break;
    }
    if (isalpha(uri[i])) {
      continue;
    }
    if (i == 0) {
      // first character must be a letter
      break;
    }
    if (isdigit(uri[i]) || uri[i] == '+' || uri[i] == '.' || uri[i] == '-') {
      continue;
    }
    if (uri[i] != ':') {
      break;
    }
    std::string scheme(uri, 0, i);
    scheme.append("://<suppressed>");
    return scheme;
  }
  return "<no-scheme URI suppressed>";
}

}  // namespace net
}  // namespace ave
