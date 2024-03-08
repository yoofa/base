/*
 * aligned_memory.h
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef ALIGNED_MEMORY_H
#define ALIGNED_MEMORY_H

#include <stddef.h>

namespace ave {
namespace base {

// Returns a pointer to the first boundry of `alignment` bytes following the
// address of `ptr`.
// Note that there is no guarantee that the memory in question is available.
// `ptr` has no requirements other than it can't be NULL.
void* GetRightAlign(const void* ptr, size_t alignment);

// Allocates memory of `size` bytes aligned on an `alignment` boundry.
// The return value is a pointer to the memory. Note that the memory must
// be de-allocated using AlignedFree.
void* AlignedMalloc(size_t size, size_t alignment);
// De-allocates memory created using the AlignedMalloc() API.
void AlignedFree(void* mem_block);

// Templated versions to facilitate usage of aligned malloc without casting
// to and from void*.
template <typename T>
T* GetRightAlign(const T* ptr, size_t alignment) {
  return reinterpret_cast<T*>(
      GetRightAlign(reinterpret_cast<const void*>(ptr), alignment));
}
template <typename T>
T* AlignedMalloc(size_t size, size_t alignment) {
  return reinterpret_cast<T*>(AlignedMalloc(size, alignment));
}

// Deleter for use with unique_ptr. E.g., use as
//   std::unique_ptr<Foo, AlignedFreeDeleter> foo;
struct AlignedFreeDeleter {
  inline void operator()(void* ptr) const { AlignedFree(ptr); }
};

}  // namespace base
}  // namespace ave

#endif /* !ALIGNED_MEMORY_H */
