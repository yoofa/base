/*
 * copy_on_write_buffer.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BASE_COPY_ON_WRITE_BUFFER_H
#define BASE_COPY_ON_WRITE_BUFFER_H

#include <cstdint>

#include <algorithm>
#include <cstring>
#include <string_view>
#include <type_traits>
#include <utility>

#include <memory>

#include "base/buffer.h"
#include "base/checks.h"
#include "base/type_traits.h"

namespace ave {
namespace base {

class CopyOnWriteBuffer {
 public:
  // An empty buffer.
  CopyOnWriteBuffer();
  // Share the data with an existing buffer.
  CopyOnWriteBuffer(const CopyOnWriteBuffer& buf);
  // Move contents from an existing buffer.
  CopyOnWriteBuffer(CopyOnWriteBuffer&& buf) noexcept;

  // Construct a buffer from a string, convenient for unittests.
  explicit CopyOnWriteBuffer(std::string_view s);

  // Construct a buffer with the specified number of uninitialized bytes.
  explicit CopyOnWriteBuffer(size_t size);
  CopyOnWriteBuffer(size_t size, size_t capacity);

  // Construct a buffer and copy the specified number of bytes into it.
  template <typename T,
            typename std::enable_if_t<internal::BufferCompat<uint8_t, T>::value,
                                      int> = 0>
  CopyOnWriteBuffer(const T* data, size_t size)
      : CopyOnWriteBuffer(data, size, size) {}

  template <typename T,
            typename std::enable_if_t<internal::BufferCompat<uint8_t, T>::value,
                                      int> = 0>
  CopyOnWriteBuffer(const T* data, size_t size, size_t capacity)
      : CopyOnWriteBuffer(size, capacity) {
    if (buffer_) {
      std::memcpy(buffer_->data(), data, size);
      offset_ = 0;
      size_ = size;
    }
  }

  // Construct a buffer from the contents of an array.
  template <typename T,
            size_t N,
            typename std::enable_if_t<internal::BufferCompat<uint8_t, T>::value,
                                      int> = 0>
  CopyOnWriteBuffer(const T (&array)[N])  // NOLINT: runtime/explicit
      : CopyOnWriteBuffer(array, N) {}

  // Construct a buffer from a vector like type.
  template <typename VecT,
            typename ElemT = typename std::remove_pointer_t<
                decltype(std::declval<VecT>().data())>,
            typename std::enable_if_t<
                !std::is_same_v<VecT, CopyOnWriteBuffer> &&
                    HasDataAndSize<VecT, ElemT>::value &&
                    internal::BufferCompat<uint8_t, ElemT>::value,
                int> = 0>
  explicit CopyOnWriteBuffer(const VecT& v)
      : CopyOnWriteBuffer(v.data(), v.size()) {}

  // Construct a buffer from a vector like type and a capacity argument
  template <typename VecT,
            typename ElemT = typename std::remove_pointer_t<
                decltype(std::declval<VecT>().data())>,
            typename std::enable_if_t<
                !std::is_same_v<VecT, CopyOnWriteBuffer> &&
                HasDataAndSize<VecT, ElemT>::value &&
                internal::BufferCompat<uint8_t, ElemT>::value>* = nullptr>
  explicit CopyOnWriteBuffer(const VecT& v, size_t capacity)
      : CopyOnWriteBuffer(v.data(), v.size(), capacity) {}

  ~CopyOnWriteBuffer();

  // Get a pointer to the data.
  template <typename T = uint8_t,
            typename std::enable_if_t<
                internal::BufferCompat<uint8_t, T>::value>* = nullptr>
  const T* data() const {
    return cdata<T>();
  }

  // Get writable pointer to the data. This will create a copy of the underlying
  // data if it is shared with other buffers.
  template <typename T = uint8_t,
            typename std::enable_if_t<internal::BufferCompat<uint8_t, T>::value,
                                      int> = 0>
  T* MutableData() {
    AVE_DCHECK(IsConsistent());
    if (!buffer_) {
      return nullptr;
    }
    UnshareAndEnsureCapacity(capacity());
    return buffer_->data<T>() + offset_;
  }

  // Get const pointer to the data.
  template <typename T = uint8_t,
            typename std::enable_if_t<internal::BufferCompat<uint8_t, T>::value,
                                      int> = 0>
  const T* cdata() const {
    AVE_DCHECK(IsConsistent());
    if (!buffer_) {
      return nullptr;
    }
    return buffer_->data<T>() + offset_;
  }

  bool empty() const { return size_ == 0; }

  size_t size() const {
    AVE_DCHECK(IsConsistent());
    return size_;
  }

  size_t capacity() const {
    AVE_DCHECK(IsConsistent());
    return buffer_ ? buffer_->capacity() - offset_ : 0;
  }

  const uint8_t* begin() const { return data(); }
  const uint8_t* end() const { return data() + size_; }

  CopyOnWriteBuffer& operator=(const CopyOnWriteBuffer& buf) {
    AVE_DCHECK(IsConsistent());
    AVE_DCHECK(buf.IsConsistent());
    if (&buf != this) {
      buffer_ = buf.buffer_;
      offset_ = buf.offset_;
      size_ = buf.size_;
    }
    return *this;
  }

  CopyOnWriteBuffer& operator=(CopyOnWriteBuffer&& buf) {
    AVE_DCHECK(IsConsistent());
    AVE_DCHECK(buf.IsConsistent());
    buffer_ = std::move(buf.buffer_);
    offset_ = buf.offset_;
    size_ = buf.size_;
    buf.offset_ = 0;
    buf.size_ = 0;
    return *this;
  }

  bool operator==(const CopyOnWriteBuffer& buf) const;

  bool operator!=(const CopyOnWriteBuffer& buf) const {
    return !(*this == buf);
  }

  uint8_t operator[](size_t index) const {
    AVE_DCHECK_LT(index, size());
    return cdata()[index];
  }

  // Replace the contents of the buffer.
  template <typename T,
            typename std::enable_if_t<
                internal::BufferCompat<uint8_t, T>::value>* = nullptr>
  void SetData(const T* data, size_t size) {
    AVE_DCHECK(IsConsistent());
    if (!buffer_) {
      buffer_ = size > 0 ? std::make_shared<Buffer>(data, size) : nullptr;
    } else if (buffer_.use_count() != 1) {
      buffer_ = std::make_shared<Buffer>(data, size, capacity());
    } else {
      buffer_->SetData(data, size);
    }
    offset_ = 0;
    size_ = size;

    AVE_DCHECK(IsConsistent());
  }

  template <typename T,
            size_t N,
            typename std::enable_if_t<
                internal::BufferCompat<uint8_t, T>::value>* = nullptr>
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  void SetData(const T (&array)[N]) {
    SetData(array, N);
  }

  void SetData(const CopyOnWriteBuffer& buf) {
    AVE_DCHECK(IsConsistent());
    AVE_DCHECK(buf.IsConsistent());
    if (&buf != this) {
      buffer_ = buf.buffer_;
      offset_ = buf.offset_;
      size_ = buf.size_;
    }
  }

  // Append data to the buffer.
  template <typename T,
            typename std::enable_if_t<
                internal::BufferCompat<uint8_t, T>::value>* = nullptr>
  void AppendData(const T* data, size_t size) {
    AVE_DCHECK(IsConsistent());
    if (!buffer_) {
      buffer_ = std::make_shared<Buffer>(data, size);
      offset_ = 0;
      size_ = size;
      AVE_DCHECK(IsConsistent());
      return;
    }

    UnshareAndEnsureCapacity(std::max(capacity(), size_ + size));

    buffer_->SetSize(offset_ + size_);  // Remove data to the right of slice.
    buffer_->AppendData(data, size);
    size_ += size;

    AVE_DCHECK(IsConsistent());
  }

  template <typename T,
            size_t N,
            typename std::enable_if_t<
                internal::BufferCompat<uint8_t, T>::value>* = nullptr>
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  void AppendData(const T (&array)[N]) {
    AppendData(array, N);
  }

  template <typename VecT,
            typename ElemT = typename std::remove_pointer_t<
                decltype(std::declval<VecT>().data())>,
            typename std::enable_if_t<
                HasDataAndSize<VecT, ElemT>::value &&
                internal::BufferCompat<uint8_t, ElemT>::value>* = nullptr>
  void AppendData(const VecT& v) {
    AppendData(v.data(), v.size());
  }

  // Sets the size of the buffer.
  void SetSize(size_t size);

  // Ensure that the buffer size can be increased to at least capacity.
  void EnsureCapacity(size_t capacity);

  // Resets the buffer to zero size without altering capacity.
  void Clear();

  // Swaps two buffers.
  friend void swap(CopyOnWriteBuffer& a, CopyOnWriteBuffer& b) {
    a.buffer_.swap(b.buffer_);
    std::swap(a.offset_, b.offset_);
    std::swap(a.size_, b.size_);
  }

  CopyOnWriteBuffer Slice(size_t offset, size_t length) const {
    CopyOnWriteBuffer slice(*this);
    AVE_DCHECK_LE(offset, size_);
    AVE_DCHECK_LE(length + offset, size_);
    slice.offset_ += offset;
    slice.size_ = length;
    return slice;
  }

 private:
  // Create a copy of the underlying data if it is referenced from other Buffer
  // objects or there is not enough capacity.
  void UnshareAndEnsureCapacity(size_t new_capacity);

  // Pre- and postcondition of all methods.
  bool IsConsistent() const {
    if (buffer_) {
      return buffer_->capacity() > 0 && offset_ <= buffer_->size() &&
             offset_ + size_ <= buffer_->size();
    }
    return size_ == 0 && offset_ == 0;
  }

  // buffer_ is either null, or points to a Buffer with capacity > 0.
  std::shared_ptr<Buffer> buffer_;
  // This buffer may represent a slice of a original data.
  size_t offset_;  // Offset of a current slice in the original data in buffer_.
  size_t size_;    // Size of a current slice in the original data in buffer_.
};

}  // namespace base
}  // namespace ave

#endif /* !BASE_COPY_ON_WRITE_BUFFER_H */
