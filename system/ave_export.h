/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef AVE_BASE_SYSTEM_AVE_EXPORT_H_
#define AVE_BASE_SYSTEM_AVE_EXPORT_H_

// AVE_EXPORT is used to mark symbols as exported or imported when WebRTC is
// built or used as a shared library.
// When WebRTC is built as a static library the AVE_EXPORT macro expands to
// nothing.

#ifdef AVE_ENABLE_SYMBOL_EXPORT

#ifdef AVE_WIN

#ifdef AVE_LIBRARY_IMPL
#define AVE_EXPORT __declspec(dllexport)
#else
#define AVE_EXPORT __declspec(dllimport)
#endif

#else  // AVE_WIN

#if __has_attribute(visibility) && defined(AVE_LIBRARY_IMPL)
#define AVE_EXPORT __attribute__((visibility("default")))
#endif

#endif  // AVE_WIN

#endif  // AVE_ENABLE_SYMBOL_EXPORT

#ifndef AVE_EXPORT
#define AVE_EXPORT
#endif

#endif  // AVE_BASE_SYSTEM_AVE_EXPORT_H_
