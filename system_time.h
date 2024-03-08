/*
 * system_time.h
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

#include <stdint.h>

namespace ave {
namespace base {

// Returns the actual system time, even if a clock is set for testing.
// Useful for timeouts while using a test clock, or for logging.
int64_t SystemTimeNanos();

}  // namespace base
}  // namespace ave

#endif /* !SYSTEM_TIME_H */
