/*
 * http_source.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "http_source.h"

#include <sstream>

#include "base/data_source/data_source_base.h"
#include "base/net/http/http_connection.h"
#include "base/net/utils.h"
#include "base/time_utils.h"

namespace ave {

namespace {
constexpr size_t kMaxReadSize = 64 * 1024;
}  // namespace

HTTPSource::HTTPSource(std::shared_ptr<net::HTTPConnection> connection)
    : init_check_(connection == nullptr ? NO_INIT : OK),
      http_connection_(std::move(connection)),
      cached_size_valid_(false),
      cached_size_(0LL) {}

HTTPSource::~HTTPSource() = default;

status_t HTTPSource::ReconnectAtOffset(off64_t offset) {
  return Connect(last_uri_.c_str(), last_headers_, offset);
}

status_t HTTPSource::Connect(
    const char* uri,
    const std::unordered_map<std::string, std::string>& headers,
    off64_t /* offset */) {
  if (init_check_ != OK) {
    return init_check_;
  }

  auto headers_copy = headers;
  if (headers_copy.find("User-Agent") == headers_copy.end()) {
    // Set a default User-Agent if it's not set.
    headers_copy["User-Agent"] = net::MakeUserAgent();
  }
  last_uri_ = uri;
  // reconnect() calls with uri == old last_uri_.c_str(), which gets zapped
  // as part of the above assignment. Ensure no accidental later use.
  uri = nullptr;

  bool success = http_connection_->Connect(last_uri_.c_str(), headers_copy);
  last_headers_ = headers_copy;
  cached_size_valid_ = false;
  if (!success) {
    return UNKNOWN_ERROR;
  }
  std::string scheme = net::UriDebugString(last_uri_);
  // TODO(youfa) use std::format when c++20 is available in the build
  // environment
  std::ostringstream oss;
  oss << "HTTPSource(" << scheme << ")";
  name_ = oss.str();
  return OK;
}

void HTTPSource::Disconnect() {
  name_ = std::string("HTTPSource<disconnected>");
  if (init_check_ != OK) {
    return;
  }
  http_connection_->Disconnect();
}

status_t HTTPSource::InitCheck() const {
  return init_check_;
}

ssize_t HTTPSource::ReadAt(off64_t offset, void* data, size_t size) {
  if (init_check_ != OK) {
    return init_check_;
  }
  int64_t start_time_us = base::TimeMicros();
  size_t num_bytes_read = 0;
  while (num_bytes_read < size) {
    size_t copy = std::min(size - num_bytes_read, kMaxReadSize);
    ssize_t n = http_connection_->ReadAt(
        offset + static_cast<off64_t>(num_bytes_read),
        static_cast<char*>(data) + num_bytes_read, copy);
    if (n < 0) {
      return n;
    }
    if (n == 0) {
      break;
    }
    num_bytes_read += n;
  }
  int64_t delay_us = base::TimeMicros() - start_time_us;
  AddBandwidthMeasurement(num_bytes_read, delay_us);
  return static_cast<ssize_t>(num_bytes_read);
}

status_t HTTPSource::GetSize(off64_t* size) {
  if (init_check_ != OK) {
    return init_check_;
  }

  if (!cached_size_valid_) {
    cached_size_ = http_connection_->GetSize();
    cached_size_valid_ = true;
  }
  *size = cached_size_;
  return *size < 0 ? UNKNOWN_ERROR : OK;
}

int32_t HTTPSource::Flags() {
  return kWantsPrefetching | kIsHTTPBasedSource;
}

void HTTPSource::Close() {
  Disconnect();
}

std::string HTTPSource::GetUri() {
  if (init_check_ != OK) {
    return {};
  }
  std::string uri;
  if (OK == http_connection_->GetUri(uri)) {
    return uri;
  }
  return last_uri_;
}

std::string HTTPSource::GetMIMEType() {
  if (init_check_ != OK) {
    return "application/octet-stream";
  }

  std::string mime_type;
  if (OK == http_connection_->GetMIMEType(mime_type)) {
    return mime_type;
  }

  return "application/octet-stream";
}

}  // namespace ave
