/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef AVE_BASE_SYSTEM_AVE_EXPORT_TEMPLATE_H_
#define AVE_BASE_SYSTEM_AVE_EXPORT_TEMPLATE_H_

// clang-format off
// clang formating would cause cpplint errors in the macros below.

// Most of this was borrowed (with minor modifications) from Chromium's
// base/export_template.h.

// Synopsis
//
// This header provides macros for using AVE_EXPORT macros with explicit
// template instantiation declarations and definitions.
// Generally, the AVE_EXPORT macros are used at declarations,
// and GCC requires them to be used at explicit instantiation declarations,
// but MSVC requires __declspec(dllexport) to be used at the explicit
// instantiation definitions instead.

// Usage
//
// In a header file, write:
//
//   extern template class AVE_EXPORT_TEMPLATE_DECLARE(AVE_EXPORT) foo<bar>;
//
// In a source file, write:
//
//   template class AVE_EXPORT_TEMPLATE_DEFINE(AVE_EXPORT) foo<bar>;

// Implementation notes
//
// On Windows, when building when AVE_EXPORT expands to __declspec(dllexport)),
// we want the two lines to expand to:
//
//     extern template class foo<bar>;
//     template class AVE_EXPORT foo<bar>;
//
// In all other cases (non-Windows, and Windows when AVE_EXPORT expands to
// __declspec(dllimport)), we want:
//
//     extern template class AVE_EXPORT foo<bar>;
//     template class foo<bar>;
//
// The implementation of this header uses some subtle macro semantics to
// detect what the provided AVE_EXPORT value was defined as and then
// to dispatch to appropriate macro definitions.  Unfortunately,
// MSVC's C preprocessor is rather non-compliant and requires special
// care to make it work.
//
// Issue 1.
//
//   #define F(x)
//   F()
//
// MSVC emits warning C4003 ("not enough actual parameters for macro
// 'F'), even though it's a valid macro invocation.  This affects the
// macros below that take just an "export" parameter, because export
// may be empty.
//
// As a workaround, we can add a dummy parameter and arguments:
//
//   #define F(x,_)
//   F(,)
//
// Issue 2.
//
//   #define F(x) G##x
//   #define Gj() ok
//   F(j())
//
// The correct replacement for "F(j())" is "ok", but MSVC replaces it
// with "Gj()".  As a workaround, we can pass the result to an
// identity macro to force MSVC to look for replacements again.  (This
// is why AVE_EXPORT_TEMPLATE_STYLE_3 exists.)

#define AVE_EXPORT_TEMPLATE_DECLARE(export)         \
  AVE_EXPORT_TEMPLATE_INVOKE(                       \
      DECLARE,                                      \
      AVE_EXPORT_TEMPLATE_STYLE(export, ), export)  // NOLINT
#define AVE_EXPORT_TEMPLATE_DEFINE(export)          \
  AVE_EXPORT_TEMPLATE_INVOKE(                       \
      DEFINE,                                       \
      AVE_EXPORT_TEMPLATE_STYLE(export, ), export)  // NOLINT

// INVOKE is an internal helper macro to perform parameter replacements
// and token pasting to chain invoke another macro.  E.g.,
//     AVE_EXPORT_TEMPLATE_INVOKE(DECLARE, DEFAULT, AVE_EXPORT)
// will export to call
//     AVE_EXPORT_TEMPLATE_DECLARE_DEFAULT(AVE_EXPORT, )
// (but with AVE_EXPORT expanded too).
#define AVE_EXPORT_TEMPLATE_INVOKE(which, style, export) \
  AVE_EXPORT_TEMPLATE_INVOKE_2(which, style, export)
#define AVE_EXPORT_TEMPLATE_INVOKE_2(which, style, export) \
  AVE_EXPORT_TEMPLATE_##which##_##style(export, )

// Default style is to apply the AVE_EXPORT macro at declaration sites.
#define AVE_EXPORT_TEMPLATE_DECLARE_DEFAULT(export, _) export
#define AVE_EXPORT_TEMPLATE_DEFINE_DEFAULT(export, _)

// The "MSVC hack" style is used when AVE_EXPORT is defined
// as __declspec(dllexport), which MSVC requires to be used at
// definition sites instead.
#define AVE_EXPORT_TEMPLATE_DECLARE_MSVC_HACK(export, _)
#define AVE_EXPORT_TEMPLATE_DEFINE_MSVC_HACK(export, _) export

// AVE_EXPORT_TEMPLATE_STYLE is an internal helper macro that identifies which
// export style needs to be used for the provided AVE_EXPORT macro definition.
// "", "__attribute__(...)", and "__declspec(dllimport)" are mapped
// to "DEFAULT"; while "__declspec(dllexport)" is mapped to "MSVC_HACK".
//
// It's implemented with token pasting to transform the __attribute__ and
// __declspec annotations into macro invocations.  E.g., if AVE_EXPORT is
// defined as "__declspec(dllimport)", it undergoes the following sequence of
// macro substitutions:
//     AVE_EXPORT_TEMPLATE_STYLE(AVE_EXPORT,)
//     AVE_EXPORT_TEMPLATE_STYLE_2(__declspec(dllimport),)
//     AVE_EXPORT_TEMPLATE_STYLE_3(
//         AVE_EXPORT_TEMPLATE_STYLE_MATCH__declspec(dllimport))
//     AVE_EXPORT_TEMPLATE_STYLE_MATCH__declspec(dllimport)
//     AVE_EXPORT_TEMPLATE_STYLE_MATCH_DECLSPEC_dllimport
//     DEFAULT
#define AVE_EXPORT_TEMPLATE_STYLE(export, _) \
  AVE_EXPORT_TEMPLATE_STYLE_2(export, )
#define AVE_EXPORT_TEMPLATE_STYLE_2(export, _) \
  AVE_EXPORT_TEMPLATE_STYLE_3(                 \
      AVE_EXPORT_TEMPLATE_STYLE_MATCH_foj3FJo5StF0OvIzl7oMxA##export)
#define AVE_EXPORT_TEMPLATE_STYLE_3(style) style

// Internal helper macros for AVE_EXPORT_TEMPLATE_STYLE.
//
// XXX: C++ reserves all identifiers containing "__" for the implementation,
// but "__attribute__" and "__declspec" already contain "__" and the token-paste
// operator can only add characters; not remove them.  To minimize the risk of
// conflict with implementations, we include "foj3FJo5StF0OvIzl7oMxA" (a random
// 128-bit string, encoded in Base64) in the macro name.
#define AVE_EXPORT_TEMPLATE_STYLE_MATCH_foj3FJo5StF0OvIzl7oMxA DEFAULT
#define AVE_EXPORT_TEMPLATE_STYLE_MATCH_foj3FJo5StF0OvIzl7oMxA__attribute__( \
    ...)                                                                     \
  DEFAULT
#define AVE_EXPORT_TEMPLATE_STYLE_MATCH_foj3FJo5StF0OvIzl7oMxA__declspec(arg) \
  AVE_EXPORT_TEMPLATE_STYLE_MATCH_DECLSPEC_##arg

// Internal helper macros for AVE_EXPORT_TEMPLATE_STYLE.
#define AVE_EXPORT_TEMPLATE_STYLE_MATCH_DECLSPEC_dllexport MSVC_HACK
#define AVE_EXPORT_TEMPLATE_STYLE_MATCH_DECLSPEC_dllimport DEFAULT

// Sanity checks.
//
// AVE_EXPORT_TEMPLATE_TEST uses the same macro invocation pattern as
// AVE_EXPORT_TEMPLATE_DECLARE and AVE_EXPORT_TEMPLATE_DEFINE do to check that
// they're working correctly. When they're working correctly, the sequence of
// macro replacements should go something like:
//
//     AVE_EXPORT_TEMPLATE_TEST(DEFAULT, __declspec(dllimport));
//
//     static_assert(AVE_EXPORT_TEMPLATE_INVOKE(TEST_DEFAULT,
//         AVE_EXPORT_TEMPLATE_STYLE(__declspec(dllimport), ),
//         __declspec(dllimport)), "__declspec(dllimport)");
//
//     static_assert(AVE_EXPORT_TEMPLATE_INVOKE(TEST_DEFAULT,
//         DEFAULT, __declspec(dllimport)), "__declspec(dllimport)");
//
//     static_assert(AVE_EXPORT_TEMPLATE_TEST_DEFAULT_DEFAULT(
//         __declspec(dllimport)), "__declspec(dllimport)");
//
//     static_assert(true, "__declspec(dllimport)");
//
// When they're not working correctly, a syntax error should occur instead.
#define AVE_EXPORT_TEMPLATE_TEST(want, export)                    \
  static_assert(                                                  \
      AVE_EXPORT_TEMPLATE_INVOKE(                                 \
          TEST_##want,                                            \
          AVE_EXPORT_TEMPLATE_STYLE(export, ), export), #export)  // NOLINT
#define AVE_EXPORT_TEMPLATE_TEST_DEFAULT_DEFAULT(...) true
#define AVE_EXPORT_TEMPLATE_TEST_MSVC_HACK_MSVC_HACK(...) true

AVE_EXPORT_TEMPLATE_TEST(DEFAULT, );  // NOLINT
AVE_EXPORT_TEMPLATE_TEST(DEFAULT, __attribute__((visibility("default"))));
AVE_EXPORT_TEMPLATE_TEST(MSVC_HACK, __declspec(dllexport));
AVE_EXPORT_TEMPLATE_TEST(DEFAULT, __declspec(dllimport));

#undef AVE_EXPORT_TEMPLATE_TEST
#undef AVE_EXPORT_TEMPLATE_TEST_DEFAULT_DEFAULT
#undef AVE_EXPORT_TEMPLATE_TEST_MSVC_HACK_MSVC_HACK

// clang-format on

#endif  // AVE_BASE_SYSTEM_AVE_EXPORT_TEMPLATE_H_
