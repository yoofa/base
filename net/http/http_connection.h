/*
 * http_connection.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include <string>
#include <unordered_map>

#include "base/constructor_magic.h"
#include "base/errors.h"

namespace ave {
namespace net {

class HTTPConnection {
 public:
  HTTPConnection() = default;
  virtual ~HTTPConnection() = default;
  virtual bool Connect(
      const char* uri,
      const std::unordered_map<std::string, std::string>& headers) = 0;

  virtual void Disconnect() = 0;
  virtual ssize_t ReadAt(off64_t offset, void* data, size_t size) = 0;
  virtual off64_t GetSize() = 0;
  virtual status_t GetMIMEType(std::string& mime_type) = 0;
  virtual status_t GetUri(std::string& uri) = 0;

 private:
  AVE_DISALLOW_COPY_AND_ASSIGN(HTTPConnection);
};

}  // namespace net
}  // namespace ave

#endif /* !HTTP_CONNECTION_H */
