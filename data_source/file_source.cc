/*
 * file_source.cc
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "file_source.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>

#include "base/attributes.h"
#include "base/logging.h"
#include "base/utils.h"

namespace ave {

FileSource::FileSource(const char* filename)
    : fd_(-1), start_offset_(0), length_(-1), offset_(0), name_("<null>") {
  if (filename) {
    name_ = std::string("FileSource(") + filename + ")";
  }
  AVE_LOG(LS_VERBOSE) << name_;
  fd_ = open(filename, O_LARGEFILE | O_RDONLY);

  if (fd_ >= 0) {
    length_ = lseek64(fd_, 0, SEEK_END);
  } else {
    AVE_LOG(LS_ERROR) << "Failed to open file" << filename << ". "
                      << strerror(errno);
  }
}

FileSource::FileSource(int fd, int64_t offset, int64_t length)
    : fd_(fd),
      start_offset_(offset),
      length_(length),
      offset_(0),
      name_("<null>") {
  AVE_LOG(LS_VERBOSE) << "fd=" << fd << ", offset=" << offset
                      << ", length=" << length;

  if (start_offset_ < 0) {
    start_offset_ = 0;
  }
  if (length_ < 0) {
    length_ = 0;
  }
  if (length_ > INT64_MAX - start_offset_) {
    length_ = INT64_MAX - start_offset_;
  }
  struct stat s = {0};
  if (fstat(fd, &s) == 0) {
    if (start_offset_ > s.st_size) {
      start_offset_ = s.st_size;
      length_ = 0;
    }
    if (start_offset_ + length_ > s.st_size) {
      length_ = s.st_size - start_offset_;
    }
  }
  if (start_offset_ != offset || length_ != length) {
    AVE_LOG(LS_WARNING) << "offset/length adjusted from" << offset << "/"
                        << length << " to " << start_offset_ << "/" << length_;
  }

  name_ = std::string("FileSource(fd(") + base::nameForFd(fd) + "), " +
          std::to_string(start_offset_) + ", " + std::to_string(length_) + ")";
}

FileSource::~FileSource() {
  if (fd_ >= 0) {
    ::close(fd_);
    fd_ = -1;
  }
}

status_t FileSource::InitCheck() const {
  return fd_ >= 0 ? OK : UNKNOWN_ERROR;
}

status_t FileSource::GetPosition(off64_t* position) {
  std::lock_guard<std::mutex> lock(lock_);
  *position = offset_;
  return OK;
}

ssize_t FileSource::Seek(off64_t position, int whence) {
  if (fd_ < 0) {
    return NO_INIT;
  }

  if (position < 0) {
    return UNKNOWN_ERROR;
  }

  if (position > length_) {
    return OK;
  }

  std::lock_guard<std::mutex> lock(lock_);
  return seek_l(position, whence);
}

ssize_t FileSource::seek_l(off64_t position, int whence AVE_MAYBE_UNUSED) {
  off64_t result = lseek64(fd_, position + start_offset_, SEEK_SET);
  if (result >= 0) {
    offset_ = result;
  }
  return offset_;
}

ssize_t FileSource::read_l(void* data, size_t size) {
  uint64_t sizeToRead = size;
  if (static_cast<int64_t>(offset_ + size) > length_) {
    sizeToRead = length_ - offset_;
  }
  ssize_t readSize = ::read(fd_, data, sizeToRead);
  if (readSize > 0) {
    offset_ += readSize;
  }
  return readSize;
}

ssize_t FileSource::Read(void* data, size_t size) {
  if (fd_ < 0) {
    return NO_INIT;
  }

  std::lock_guard<std::mutex> lock(lock_);
  return read_l(data, size);
}

ssize_t FileSource::ReadAt(off64_t offset, void* data, size_t size) {
  if (fd_ < 0) {
    return NO_INIT;
  }
  std::lock_guard<std::mutex> lock(lock_);

  ssize_t seekOffset = seek_l(offset, SEEK_SET);

  if (seekOffset < 0) {
    return seekOffset;
  }

  return read_l(data, size);
}

status_t FileSource::GetSize(off64_t* size) {
  std::lock_guard<std::mutex> lock(lock_);

  if (fd_ < 0) {
    return NO_INIT;
  }

  *size = length_;

  return OK;
}

}  // namespace ave
