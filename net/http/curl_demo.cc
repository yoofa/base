/*
 * curl_demo.cc
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include <curl/curl.h>
#include <array>
#include <iostream>
#include <memory>
#include <string>

#include "base/net/http/curl_http_provider.h"

void PrintUsage() {
  std::cout << "Usage: ave_curl <url>" << std::endl;
  std::cout << "Example: ave_curl https://www.google.com" << std::endl;
}

bool EnsureScheme(std::string& url) {
  if (url.find("://") == std::string::npos) {
    url = "http://" + url;
  }
  return true;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    PrintUsage();
    return 1;
  }

  std::string url = argv[1];
  EnsureScheme(url);

  try {
    // Create provider and connection
    ave::net::CurlHttpProvider provider;
    auto connection = provider.CreateConnection();

    if (!connection) {
      std::cerr << "Failed to create HTTP connection" << std::endl;
      return 1;
    }

    // Add some basic headers like curl does
    std::unordered_map<std::string, std::string> headers = {
        {"User-Agent", "ave_curl/1.0"}, {"Accept", "*/*"}};

    // Connect to the URL
    if (!connection->Connect(url.c_str(), headers)) {
      std::cerr << "Failed to connect to " << url << std::endl;
      return 1;
    }

    // Get and print response info
    std::string mime_type;
    if (connection->GetMIMEType(mime_type) == 0) {
      std::cout << "Content-Type: " << mime_type << std::endl;
    }

    off64_t size = connection->GetSize();
    if (size >= 0) {
      std::cout << "Content-Length: " << size << std::endl;
    }

    // Read and print the content
    const size_t buffer_size = 4096;
    std::array<char, buffer_size> buffer{};
    off64_t offset = 0;

    while (true) {
      ssize_t bytes_read =
          connection->ReadAt(offset, buffer.data(), buffer_size);
      if (bytes_read <= 0) {
        break;
      }

      std::cout.write(buffer.data(), bytes_read);
      offset += bytes_read;
    }

    connection->Disconnect();

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
