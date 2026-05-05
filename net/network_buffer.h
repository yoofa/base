/*
 * network_buffer.h
 * Copyright (C) 2024 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BASE_NET_NETWORK_BUFFER_H
#define BASE_NET_NETWORK_BUFFER_H

#include "base/buffer.h"

namespace ave {
namespace base {
namespace net {

// NetworkBuffer is an alias for base::Buffer, providing efficient
// dynamic buffer management with move semantics for network I/O.
using NetworkBuffer = base::Buffer;

}  // namespace net
}  // namespace base
}  // namespace ave

#endif /* !BASE_NET_NETWORK_BUFFER_H */
