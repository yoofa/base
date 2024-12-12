/*
 * curl_http_connection.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/net/http/curl_http_connection.h"

#include <algorithm>
#include <cstring>

namespace ave {
namespace net {

CurlHttpConnection::CurlHttpConnection()
    : curl_(nullptr), content_length_(-1), connected_(false) {
  curl_ = curl_easy_init();
}

CurlHttpConnection::~CurlHttpConnection() {
  Disconnect();
  if (curl_) {
    curl_easy_cleanup(curl_);
    curl_ = nullptr;
  }
}

bool CurlHttpConnection::Connect(
    const char* uri,
    const std::unordered_map<std::string, std::string>& headers) {
  if (!curl_) {
    return false;
  }

  Reset();
  uri_ = uri;

  curl_easy_setopt(curl_, CURLOPT_URL, uri);
  curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, HeaderCallback);
  curl_easy_setopt(curl_, CURLOPT_HEADERDATA, this);
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, this);

  // Add custom headers
  struct curl_slist* chunk = nullptr;
  for (const auto& header : headers) {
    std::string header_line = header.first + ": " + header.second;
    chunk = curl_slist_append(chunk, header_line.c_str());
  }
  if (chunk) {
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, chunk);
  }

  // Perform request
  CURLcode res = curl_easy_perform(curl_);
  if (chunk) {
    curl_slist_free_all(chunk);
  }

  if (res != CURLE_OK) {
    return false;
  }

  connected_ = ParseHeaders();
  return connected_;
}

void CurlHttpConnection::Disconnect() {
  if (curl_) {
    curl_easy_reset(curl_);
  }
  Reset();
}

ssize_t CurlHttpConnection::ReadAt(off64_t offset, void* data, size_t size) {
  if (!connected_ || offset >= content_length_) {
    return -1;
  }

  size_t available =
      std::min(size, buffer_.size() - static_cast<size_t>(offset));
  if (available > 0) {
    memcpy(data, buffer_.data() + offset, available);
  }
  return static_cast<ssize_t>(available);
}

off64_t CurlHttpConnection::GetSize() {
  return content_length_;
}

status_t CurlHttpConnection::GetMIMEType(std::string& mime_type) {
  if (!connected_) {
    return -1;
  }
  mime_type = mime_type_;
  return 0;
}

status_t CurlHttpConnection::GetUri(std::string& uri) {
  if (!connected_) {
    return -1;
  }
  uri = uri_;
  return 0;
}

size_t CurlHttpConnection::HeaderCallback(char* buffer,
                                          size_t size,
                                          size_t nitems,
                                          void* userdata) {
  size_t real_size = size * nitems;
  auto* conn = static_cast<CurlHttpConnection*>(userdata);
  conn->headers_.append(buffer, real_size);
  return real_size;
}

size_t CurlHttpConnection::WriteCallback(char* ptr,
                                         size_t size,
                                         size_t nmemb,
                                         void* userdata) {
  size_t real_size = size * nmemb;
  auto* conn = static_cast<CurlHttpConnection*>(userdata);
  conn->buffer_.insert(conn->buffer_.end(), ptr, ptr + real_size);
  return real_size;
}

void CurlHttpConnection::Reset() {
  uri_.clear();
  mime_type_.clear();
  content_length_ = -1;
  buffer_.clear();
  headers_.clear();
  connected_ = false;
}

bool CurlHttpConnection::ParseHeaders() {
  if (headers_.empty()) {
    return false;
  }

  // Get Content-Type
  char* content_type = nullptr;
  curl_easy_getinfo(curl_, CURLINFO_CONTENT_TYPE, &content_type);
  if (content_type) {
    mime_type_ = content_type;
  }

  // Get Content-Length
  double cl{};
  if (curl_easy_getinfo(curl_, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &cl) ==
      CURLE_OK) {
    content_length_ = static_cast<off64_t>(cl);
  }

  return true;
}

}  // namespace net
}  // namespace ave
