/*
 * zero_memory.h
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef ZERO_MEMORY_H
#define ZERO_MEMORY_H

#include <stddef.h>

#include <type_traits>

#include "array_view.h"

namespace avp {
namespace base {

// Fill memory with zeros in a way that the compiler doesn't optimize it away
// even if the pointer is not used afterwards.
void ExplicitZeroMemory(void* ptr, size_t len);

template <typename T,
          typename std::enable_if<!std::is_const<T>::value &&
                                  std::is_trivial<T>::value>::type* = nullptr>
void ExplicitZeroMemory(ArrayView<T> a) {
  ExplicitZeroMemory(a.data(), a.size());
}

}  // namespace base
}  // namespace avp

#endif /* !ZERO_MEMORY_H */
