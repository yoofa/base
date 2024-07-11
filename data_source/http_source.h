/*
 * http_source.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef HTTP_SOURCE_H
#define HTTP_SOURCE_H

#include <memory>
#include <string>
#include <unordered_map>

#include "base/constructor_magic.h"
#include "data_source.h"
#include "http_base.h"

namespace ave {

namespace net {
class HTTPConnection;
}

class HTTPSource : public DataSource, public HTTPBase {
  HTTPSource(std::shared_ptr<net::HTTPConnection> connection);

  virtual status_t ReconnectAtOffset(off64_t offset);

  // HTTPBase interfaces
  status_t Connect(const char* uri,
                   const std::unordered_map<std::string, std::string>& headers,
                   off64_t offset) override;
  void Disconnect() override;

  // DataSourceBase interfaces
  status_t InitCheck() const override;
  ssize_t ReadAt(off64_t offset, void* data, size_t size) override;
  status_t GetSize(off64_t* size) override;
  uint64_t Flags() override;
  void Close() override;

 protected:
  ~HTTPSource() override;

  // DataSource interfaces
  std::string GetUri() override;
  virtual std::string GetMIMEType();

  std::string last_uri_;

 private:
  status_t init_check_;
  std::shared_ptr<net::HTTPConnection> http_connection_;

  std::unordered_map<std::string, std::string> last_headers_;

  bool cached_size_valid_;
  off64_t cached_size_;

  AVE_DISALLOW_COPY_AND_ASSIGN(HTTPSource);
};

}  // namespace ave

#endif /* !HTTP_SOURCE_H */
