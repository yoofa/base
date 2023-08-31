/*
 * zero_memory.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "zero_memory.h"

#include <string.h>
#include "checks.h"

namespace avp {

// Code and comment taken from "OPENSSL_cleanse" of BoringSSL.
void ExplicitZeroMemory(void* ptr, size_t len) {
  DCHECK(ptr || !len);
  memset(ptr, 0, len);
#if !defined(__pnacl__)
  /* As best as we can tell, this is sufficient to break any optimisations that
     might try to eliminate "superfluous" memsets. If there's an easy way to
     detect memset_s, it would be better to use that. */
  __asm__ __volatile__("" : : "r"(ptr) : "memory");  // NOLINT
#endif
}

}  // namespace avp
