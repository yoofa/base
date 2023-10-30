/*
 * attributes.h
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

// AVP_HAVE_ATTRIBUTE
//
// A function-like feature checking macro that is a wrapper around
// `__has_attribute`, which is defined by GCC 5+ and Clang and evaluates to a
// nonzero constant integer if the attribute is supported or 0 if not.
//
// It evaluates to zero if `__has_attribute` is not defined by the compiler.
//
// GCC: https://gcc.gnu.org/gcc-5/changes.html
// Clang: https://clang.llvm.org/docs/LanguageExtensions.html
#ifdef __has_attribute
#define AVP_HAVE_ATTRIBUTE(x) __has_attribute(x)
#else
#define AVP_HAVE_ATTRIBUTE(x) 0
#endif

// -----------------------------------------------------------------------------
// Function Attributes
// -----------------------------------------------------------------------------
//

// AVP_HAVE_CPP_ATTRIBUTE
//
// A function-like feature checking macro that accepts C++11 style attributes.
// It's a wrapper around `__has_cpp_attribute`, defined by ISO C++ SD-6
// (https://en.cppreference.com/w/cpp/experimental/feature_test). If we don't
// find `__has_cpp_attribute`, will evaluate to 0.
#if defined(__cplusplus) && defined(__has_cpp_attribute)
// NOTE: requiring __cplusplus above should not be necessary, but
// works around https://bugs.llvm.org/show_bug.cgi?id=23435.
#define AVP_HAVE_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define AVP_HAVE_CPP_ATTRIBUTE(x) 0
#endif

// AVP_NORETURN
//
// Tells the compiler that a given function never returns.
#if AVP_HAVE_ATTRIBUTE(noreturn) || (defined(__GNUC__) && !defined(__clang__))
#define AVP_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define AVP_NORETURN __declspec(noreturn)
#else
#define AVP_NORETURN
#endif

// AVP_FALLTHROUGH_INTENDED
//
// Annotates implicit fall-through between switch labels, allowing a case to
// indicate intentional fallthrough and turn off warnings about any lack of a
// `break` statement. The AVP_FALLTHROUGH_INTENDED macro should be followed by
// a semicolon and can be used in most places where `break` can, provided that
// no statements exist between it and the next switch label.
//
// Example:
//
//  switch (x) {
//    case 40:
//    case 41:
//      if (truth_is_out_there) {
//        ++x;
//        AVP_FALLTHROUGH_INTENDED;  // Use instead of/along with annotations
//                                    // in comments
//      } else {
//        return x;
//      }
//    case 42:
//      ...
//
// Notes: When supported, GCC and Clang can issue a warning on switch labels
// with unannotated fallthrough using the warning `-Wimplicit-fallthrough`. See
// clang documentation on language extensions for details:
// https://clang.llvm.org/docs/AttributeReference.html#fallthrough-clang-fallthrough
//
// When used with unsupported compilers, the AVP_FALLTHROUGH_INTENDED macro has
// no effect on diagnostics. In any case this macro has no effect on runtime
// behavior and performance of code.

#ifdef AVP_FALLTHROUGH_INTENDED
#error "AVP_FALLTHROUGH_INTENDED should not be defined."
#elif AVP_HAVE_CPP_ATTRIBUTE(fallthrough)
#define AVP_FALLTHROUGH_INTENDED [[fallthrough]]
#elif AVP_HAVE_CPP_ATTRIBUTE(clang::fallthrough)
#define AVP_FALLTHROUGH_INTENDED [[clang::fallthrough]]
#elif AVP_HAVE_CPP_ATTRIBUTE(gnu::fallthrough)
#define AVP_FALLTHROUGH_INTENDED [[gnu::fallthrough]]
#else
#define AVP_FALLTHROUGH_INTENDED \
  do {                           \
  } while (0)
#endif

// AVP_NODISCARD
//
// Tells the compiler to warn about unused results.
//
// When annotating a function, it must appear as the first part of the
// declaration or definition. The compiler will warn if the return value from
// such a function is unused:
//
//   AVP_NODISCARD Sprocket* AllocateSprocket();
//   AllocateSprocket();  // Triggers a warning.
//
// When annotating a class, it is equivalent to annotating every function which
// returns an instance.
//
//   class AVP_NODISCARD Sprocket {};
//   Sprocket();  // Triggers a warning.
//
//   Sprocket MakeSprocket();
//   MakeSprocket();  // Triggers a warning.
//
// Note that references and pointers are not instances:
//
//   Sprocket* SprocketPointer();
//   SprocketPointer();  // Does *not* trigger a warning.
//
// AVP_NODISCARD allows using cast-to-void to suppress the unused result
// warning. For that, warn_unused_result is used only for clang but not for gcc.
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66425
//
// Note: past advice was to place the macro after the argument list.
#if AVP_HAVE_ATTRIBUTE(nodiscard)
#define AVP_NODISCARD [[nodiscard]]
#elif defined(__clang__) && AVP_HAVE_ATTRIBUTE(warn_unused_result)
#define AVP_NODISCARD __attribute__((warn_unused_result))
#else
#define AVP_NODISCARD
#endif

// AVP_LIKELY
#if AVP_HAVE_ATTRIBUTE(likely)
#define AVP_LIKELY __attribute__((likely))
#else
#define AVP_LIKELY
#endif

// AVP_UN_LIKELY
#if AVP_HAVE_ATTRIBUTE(unlikely)
#define AVP_UNLIKELY __attribute__((unlikely))
#else
#define AVP_UNLIKELY
#endif

// -----------------------------------------------------------------------------
// Variable Attributes
// -----------------------------------------------------------------------------
//

// AVP_MAYBE_UNUSED
//
// Prevents the compiler from complaining about variables that appear unused.
//
// For code or headers that are assured to only build with C++17 and up, prefer
// just using the standard '[[maybe_unused]]' directly over this macro.
//
// Due to differences in positioning requirements between the old, compiler
// specific __attribute__ syntax and the now standard [[maybe_unused]], this
// macro does not attempt to take advantage of '[[maybe_unused]]'.
#if AVP_HAVE_ATTRIBUTE(maybe_unused) || \
    (defined(__GNUC__) && !defined(__clang__))
#undef AVP_MAYBE_UNUSED
#define AVP_MAYBE_UNUSED __attribute__((__maybe_unused__))
#else
#define AVP_MAYBE_UNUSED
#endif

#endif /* !ATTRIBUTES_H */
