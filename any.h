/*
 * any.h
 * Copyright (C) 2021 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVE_ANY_H
#define AVE_ANY_H

#include <memory>
#include <utility>

namespace ave {
namespace base {

// Deprecated, use std::any maybe better
class Any {
 public:
  Any() = default;
  virtual ~Any() = default;

  template <typename T, typename... Args>
  void Set(Args&&... args) {
    mData.reset(new T(std::forward<Args>(args)...),
                [](void* ptr) { delete static_cast<T*>(ptr); });
  }

  template <typename T>
  T* Get() {
    if (!mData) {
      return nullptr;
    }
    T* ptr = static_cast<T*>(mData.get());
    return ptr;
  }

  operator bool() { return mData.operator bool(); }

  bool empty() { return !operator bool(); }

 private:
  std::shared_ptr<void> mData;
};
}  // namespace base
}  // namespace ave

#endif /* !AVE_ANY_H */
