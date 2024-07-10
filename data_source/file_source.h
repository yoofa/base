/*
 * file_source.h
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef FILE_SOURCE_H
#define FILE_SOURCE_H

#include <mutex>
#include <string>

#include "data_source.h"

namespace ave {

class FileSource : public DataSource {
 public:
  FileSource(const char* filename);
  FileSource(int fd, int64_t offset, int64_t length);
  ~FileSource() override;

  status_t InitCheck() const override;

  ssize_t Read(void* data, size_t size) override;

  ssize_t ReadAt(off64_t offset, void* data, size_t size) override;

  status_t GetPosition(off64_t* position) override;

  ssize_t Seek(off64_t position, int whence) override;

  status_t GetSize(off64_t* size) override;

  uint64_t Flags() override { return kIsLocalFileSource | kSeekable; }

  virtual std::string toString() { return name_; }

 protected:
  ssize_t read_l(void* data, size_t size);
  ssize_t seek_l(off64_t position, int whence);
  ssize_t readAt_l(off64_t offset, void* data, size_t size);

  int fd_;
  int64_t start_offset_;
  int64_t length_;
  int64_t offset_;
  std::mutex lock_;

 private:
  std::string name_;

  FileSource(const FileSource&);
  FileSource& operator=(const FileSource&);
};

}  // namespace ave

#endif /* !FILE_SOURCE_H */
