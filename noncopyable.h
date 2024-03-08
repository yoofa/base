/*
 * noncopyable.h
 * Copyright (C) 2021 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVE_NONCOPYABLE_H
#define AVE_NONCOPYABLE_H

namespace ave {
class noncopyable {
 public:
  noncopyable(const noncopyable&) = delete;
  void operator=(const noncopyable) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

}  // namespace ave

#endif /* !AVE_NONCOPYABLE_H */
