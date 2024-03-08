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

class Any {
 public:
  Any() = default;
  virtual ~Any() = default;

  template <typename T, typename... Args>
  void Set(Args&&... args) {
    mData.reset(new T(std::forward<Args>(args)...),
                [](void* ptr) { delete (T*)ptr; });
  }

  template <typename T>
  T& Get() {
    if (!mData) {
      return nullptr;
    }
    T* ptr = (T*)mData.get();
    return *ptr;
  }

  operator bool() { return mData.operator bool(); }

  bool Empty() { return !bool(); }

 private:
  std::shared_ptr<void> mData;
};

#endif /* !AVE_ANY_H */
