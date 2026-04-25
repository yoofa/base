/*
 * utils.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace ave {
namespace base {
namespace net {

std::string MakeUserAgent();
std::string UriDebugString(const std::string& uri, bool incognito = false);

}  // namespace net
}  // namespace base
}  // namespace ave

#endif /* !UTILS_H */
