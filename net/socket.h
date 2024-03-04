/*
 * socket.h
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVP_BASE_NET_SOCKET_H
#define AVP_BASE_NET_SOCKET_H

#include <errno.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "base/net/socket_address.h"
#include "base/third_party/sigslot/sigslot.h"

namespace avp {
namespace base {
namespace net {

inline bool IsBlockingError(int e) {
  return (e == EWOULDBLOCK) || (e == EAGAIN) || (e == EINPROGRESS);
}

// General interface for the socket implementations of various networks.  The
// methods match those of normal UNIX sockets very closely.
class Socket {
 public:
  virtual ~Socket() {}

  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;

  // Returns the address to which the socket is bound.  If the socket is not
  // bound, then the any-address is returned.
  virtual SocketAddress GetLocalAddress() const = 0;

  // Returns the address to which the socket is connected.  If the socket is
  // not connected, then the any-address is returned.
  virtual SocketAddress GetRemoteAddress() const = 0;

  virtual int Bind(const SocketAddress& addr) = 0;
  virtual int Connect(const SocketAddress& addr) = 0;
  virtual int Send(const void* pv, size_t cb) = 0;
  virtual int SendTo(const void* pv, size_t cb, const SocketAddress& addr) = 0;
  // `timestamp` is in units of microseconds.
  virtual int Recv(void* pv, size_t cb, int64_t* timestamp) = 0;
  virtual int RecvFrom(void* pv,
                       size_t cb,
                       SocketAddress* paddr,
                       int64_t* timestamp) = 0;
  virtual int Listen(int backlog) = 0;
  virtual Socket* Accept(SocketAddress* paddr) = 0;
  virtual int Close() = 0;
  virtual int GetError() const = 0;
  virtual void SetError(int error) = 0;
  inline bool IsBlocking() const { return IsBlockingError(GetError()); }

  enum ConnState { CS_CLOSED, CS_CONNECTING, CS_CONNECTED };
  virtual ConnState GetState() const = 0;

  enum Option {
    OPT_DONTFRAGMENT,
    OPT_RCVBUF,                // receive buffer size
    OPT_SNDBUF,                // send buffer size
    OPT_NODELAY,               // whether Nagle algorithm is enabled
    OPT_IPV6_V6ONLY,           // Whether the socket is IPv6 only.
    OPT_DSCP,                  // DSCP code
    OPT_RTP_SENDTIME_EXTN_ID,  // This is a non-traditional socket option param.
                               // This is specific to libjingle and will be used
                               // if SendTime option is needed at socket level.
  };
  virtual int GetOption(Option opt, int* value) = 0;
  virtual int SetOption(Option opt, int value) = 0;

  // SignalReadEvent and SignalWriteEvent use multi_threaded_local to allow
  // access concurrently from different thread.
  // For example SignalReadEvent::connect will be called in AsyncUDPSocket ctor
  // but at the same time the SocketDispatcher may be signaling the read event.
  // ready to read
  sigslot::signal1<Socket*, sigslot::multi_threaded_local> SignalReadEvent;
  // ready to write
  sigslot::signal1<Socket*, sigslot::multi_threaded_local> SignalWriteEvent;
  sigslot::signal1<Socket*> SignalConnectEvent;     // connected
  sigslot::signal2<Socket*, int> SignalCloseEvent;  // closed

 protected:
  Socket() {}
};

}  // namespace net
}  // namespace base
}  // namespace avp

#endif /* !AVP_BASE_NET_SOCKET_H */
