/*
 * data_source.h
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef DATA_SOURCE_H
#define DATA_SOURCE_H

#include <string>

#include "data_source_base.h"

namespace ave {
class DataSource : public DataSourceBase {
 public:
  virtual std::string GetUri() { return {}; }

  bool GetUri(char* uri, size_t size) override {
    int ret = snprintf(uri, size, "%s", GetUri().c_str());
    return ret >= 0 && static_cast<size_t>(ret) < size;
  }

  virtual std::string GetMimeType() { return {"application/octet-stream"}; }
};
}  // namespace ave

#endif /* !DATA_SOURCE_H */
