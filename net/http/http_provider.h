/*
 * http_provider.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef HTTP_PROVIDER_H
#define HTTP_PROVIDER_H

#include <memory>
#include <string>

#include "base/net/http/http_connection.h"

namespace ave {
namespace net {

class HTTPProvider {
 public:
  HTTPProvider() = default;
  virtual ~HTTPProvider() = default;

  // Creates a new HTTP connection instance
  virtual std::shared_ptr<HTTPConnection> CreateConnection() = 0;

  // Checks if the given URI scheme is supported by this provider
  virtual bool SupportsScheme(const std::string& scheme) = 0;

 private:
  AVE_DISALLOW_COPY_AND_ASSIGN(HTTPProvider);
};

}  // namespace net
}  // namespace ave

#endif /* !HTTP_PROVIDER_H */
