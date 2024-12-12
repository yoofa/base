/*
 * curl_http_connection.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef CURL_HTTP_CONNECTION_H
#define CURL_HTTP_CONNECTION_H

#include <curl/curl.h>
#include <string>
#include <vector>

#include "base/net/http/http_connection.h"

namespace ave {
namespace net {

class CurlHttpConnection : public HTTPConnection {
 public:
  CurlHttpConnection();
  ~CurlHttpConnection() override;

  // HTTPConnection implementation
  bool Connect(
      const char* uri,
      const std::unordered_map<std::string, std::string>& headers) override;
  void Disconnect() override;
  ssize_t ReadAt(off64_t offset, void* data, size_t size) override;
  off64_t GetSize() override;
  status_t GetMIMEType(std::string& mime_type) override;
  status_t GetUri(std::string& uri) override;

 private:
  static size_t HeaderCallback(char* buffer,
                               size_t size,
                               size_t nitems,
                               void* userdata);
  static size_t WriteCallback(char* ptr,
                              size_t size,
                              size_t nmemb,
                              void* userdata);
  void Reset();
  bool ParseHeaders();

  CURL* curl_;
  std::string uri_;
  std::string mime_type_;
  off64_t content_length_;
  std::vector<char> buffer_;
  bool connected_;
  std::string headers_;

  AVE_DISALLOW_COPY_AND_ASSIGN(CurlHttpConnection);
};

}  // namespace net
}  // namespace ave

#endif /* !CURL_HTTP_CONNECTION_H */
