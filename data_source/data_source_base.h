/*
 * data_source_base.h
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef DATA_SOURCE_BASE_H
#define DATA_SOURCE_BASE_H

#include <arpa/inet.h>

#include <array>

#include "base/byte_utils.h"
#include "base/errors.h"

namespace ave {

class DataSourceBase {
 public:
  enum {
    kIsDefault = 0,
    kWantsPrefetching = 1,
    kStreamedFromLocalHost = 2,
    kIsCachingDataSource = 4,
    kIsHTTPBasedSource = 8,
    kIsLocalFileSource = 16,

    kSeekable = 32,
  };

  DataSourceBase() = default;
  virtual ~DataSourceBase() = default;

  virtual status_t InitCheck() const = 0;

  virtual ssize_t Read(void* /* data */, size_t /* size */) {
    return INVALID_OPERATION;
  }

  // read from start offset
  virtual ssize_t ReadAt(off64_t offset, void* data, size_t size) = 0;

  virtual ssize_t Seek(off64_t /* position */, int /* whence */) {
    return INVALID_OPERATION;
  }
  virtual status_t GetPosition(off64_t* /* position */) {
    return INVALID_OPERATION;
  }

  virtual status_t GetSize(off64_t* size) {
    *size = 0;
    return NO_INIT;
  }

  virtual bool GetUri(char* /*uriString*/, size_t /*bufferSize*/) {
    return false;
  }

  virtual int32_t Flags() { return kIsDefault; }

  virtual void Close() {}

  virtual status_t GetAvailableSize(off64_t /*offset*/, off64_t* /*size*/) {
    return NO_INIT;
  }

  // Convenience methods:
  bool GetUInt16(off64_t offset, uint16_t* x) {
    *x = 0;

    std::array<uint8_t, 2> byte{0};
    if (ReadAt(offset, byte.data(), 2) != 2) {
      return false;
    }

    *x = (byte[0] << 8) | byte[1];

    return true;
  }
  // 3 byte int, returned as a 32-bit int
  bool GetUInt24(off64_t offset, uint32_t* x) {
    *x = static_cast<uint32_t>(0);

    std::array<uint8_t, 3> byte{0};
    if (ReadAt(offset, byte.data(), 3) != 3) {
      return false;
    }

    *x = static_cast<uint32_t>((byte[0] << 16) | (byte[1] << 8) | byte[2]);

    return true;
  }
  bool GetUInt32(off64_t offset, uint32_t* x) {
    *x = static_cast<uint32_t>(0);

    auto tmp = static_cast<uint32_t>(0);
    if (ReadAt(offset, &tmp, 4) != 4) {
      return false;
    }

    *x = static_cast<uint32_t>(ntohl(tmp));

    return true;
  }
  bool GetUInt64(off64_t offset, uint64_t* x) {
    *x = 0;

    uint64_t tmp = 0;
    if (ReadAt(offset, &tmp, 8) != 8) {
      return false;
    }

    *x = ntoh64(tmp);

    return true;
  }

  // read either int<N> or int<2N> into a uint<2N>_t, size is the int size in
  // bytes.
  bool GetUInt16Var(off64_t offset, uint16_t* x, size_t size) {
    if (size == 2) {
      return GetUInt16(offset, x);
    }
    if (size == 1) {
      uint8_t tmp = 0;
      if (ReadAt(offset, &tmp, 1) == 1) {
        *x = tmp;
        return true;
      }
    }
    return false;
  }
  bool GetUInt32Var(off64_t offset, uint32_t* x, size_t size) {
    if (size == 4) {
      return GetUInt32(offset, x);
    }
    if (size == 2) {
      uint16_t tmp = 0;
      if (GetUInt16(offset, &tmp)) {
        *x = static_cast<uint32_t>(tmp);
        return true;
      }
    }
    return false;
  }
  bool GetUInt64Var(off64_t offset, uint64_t* x, size_t size) {
    if (size == 8) {
      return GetUInt64(offset, x);
    }
    if (size == 4) {
      auto tmp = static_cast<uint32_t>(0);
      if (GetUInt32(offset, &tmp)) {
        *x = tmp;
        return true;
      }
    }
    return false;
  }
};

}  // namespace ave

#endif /* !DATA_SOURCE_BASE_H */
