/*
 * curl_http_provider.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef CURL_HTTP_PROVIDER_H
#define CURL_HTTP_PROVIDER_H

#include "base/net/http/http_provider.h"

namespace ave {
namespace net {

class CurlHttpProvider : public HTTPProvider {
 public:
  CurlHttpProvider();
  ~CurlHttpProvider() override;

  // HTTPProvider implementation
  std::shared_ptr<HTTPConnection> CreateConnection() override;
  bool SupportsScheme(const std::string& scheme) override;

  AVE_DISALLOW_COPY_AND_ASSIGN(CurlHttpProvider);
};

}  // namespace net
}  // namespace ave

#endif /* !CURL_HTTP_PROVIDER_H */
