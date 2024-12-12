/*
 * curl_http_provider.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include <curl/curl.h>

#include "base/net/http/curl_http_connection.h"
#include "base/net/http/curl_http_provider.h"

namespace ave {
namespace net {

namespace {

bool Initialize() {
  CURLcode curl_ret = curl_global_init(CURL_GLOBAL_ALL);
  return curl_ret == CURLE_OK;
}

void Cleanup() {
  curl_global_cleanup();
}

}  // namespace

CurlHttpProvider::CurlHttpProvider() {
  if (!Initialize()) {
    throw std::runtime_error("Failed to initialize CURL");
  }
}

CurlHttpProvider::~CurlHttpProvider() {
  Cleanup();
}

std::shared_ptr<HTTPConnection> CurlHttpProvider::CreateConnection() {
  return std::make_shared<CurlHttpConnection>();
}

bool CurlHttpProvider::SupportsScheme(const std::string& scheme) {
  // Support both HTTP and HTTPS schemes
  return scheme == "http" || scheme == "https";
}

}  // namespace net
}  // namespace ave
