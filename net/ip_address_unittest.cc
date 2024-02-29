/*
 * ip_address_unittest.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/net/ip_address.h"
#include <netinet/in.h>
#include "test/gtest.h"
#include "third_party/openssl/include/openssl/x509v3.h"

namespace avp {
namespace base {
namespace net {

namespace {
static const unsigned int kIPv4PublicAddr = 0x01020304;
static const std::string kIPv4LoopbackString = "127.0.0.1";
static const std::string kIPv4AnyString = "0.0.0.0";
static const std::string kIPv4PublicAddrString = "1.2.3.4";

static const in6_addr kIPv6PublicAddr = {
    {{0x24, 0x01, 0xfa, 0x00, 0x00, 0x04, 0x10, 0x00, 0xbe, 0x30, 0x5b, 0xff,
      0xfe, 0xe5, 0x00, 0xc3}}};
static const in6_addr kIPv6PublicAddr2 = {
    {{0x24, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0xbe, 0x30, 0x5b, 0xff,
      0xfe, 0xe5, 0x00, 0xc3}}};
static const std::string kIPv6LoopbackString = "::1";
static const std::string kIPv6AnyString = "::";
static const std::string kIPv6PublicAddrString =
    "2401:fa00:4:1000:be30:5bff:fee5:c3";
static const std::string kIPv6PublicAddr2String =
    "2401::1000:be30:5bff:fee5:c3";

bool IPAddressEq(const IPAddress& ip1, const IPAddress& ip2) {
  if ((IPIsAny(ip1) != IPIsAny(ip2)) ||
      (IPIsLoopback(ip1) != IPIsLoopback(ip2)) ||
      (ip1.family() != ip2.family()) || (ip1.ToString() != ip2.ToString())) {
    return false;
  }
  in_addr ipv4_1 = ip1.ipv4();
  in_addr ipv4_2 = ip2.ipv4();
  if (memcmp(&ipv4_1, &ipv4_2, sizeof(ipv4_1)) != 0) {
    return false;
  }
  in6_addr ipv6_1 = ip1.ipv6();
  in6_addr ipv6_2 = ip2.ipv6();
  if (memcmp(&ipv6_1, &ipv6_2, sizeof(ipv6_1)) != 0) {
    return false;
  }
  return true;
}

}  // namespace

TEST(IPAddressTest, TestDefaultConstructor) {
  IPAddress ip;
  EXPECT_FALSE(IPIsAny(ip));
  EXPECT_FALSE(IPIsLoopback(ip));

  EXPECT_EQ(AF_UNSPEC, ip.family());
  EXPECT_EQ("", ip.ToString());
}

TEST(IPAddressTest, TestInAddrConstructor) {
  in_addr ipv4;
  ipv4.s_addr = htonl(INADDR_LOOPBACK);

  IPAddress ip(ipv4);
  EXPECT_TRUE(IPIsLoopback(ip));
  EXPECT_FALSE(IPIsAny(ip));
  EXPECT_EQ(AF_INET, ip.family());
  EXPECT_EQ(kIPv4LoopbackString, ip.ToString());

  ipv4.s_addr = htonl(INADDR_ANY);
  ip = IPAddress(ipv4);
  EXPECT_TRUE(IPIsAny(ip));
  EXPECT_FALSE(IPIsLoopback(ip));
  EXPECT_EQ(AF_INET, ip.family());
  EXPECT_EQ(kIPv4AnyString, ip.ToString());
}

TEST(IPAddressTest, TestIn6AddrConstructor) {
  in6_addr ipv6 = IN6ADDR_LOOPBACK_INIT;

  IPAddress ip(ipv6);
  EXPECT_TRUE(IPIsLoopback(ip));
  EXPECT_FALSE(IPIsAny(ip));
  EXPECT_EQ(AF_INET6, ip.family());
  EXPECT_EQ(kIPv6LoopbackString, ip.ToString());

  ipv6 = IN6ADDR_ANY_INIT;
  ip = IPAddress(ipv6);
  EXPECT_TRUE(IPIsAny(ip));
  EXPECT_FALSE(IPIsLoopback(ip));
  EXPECT_EQ(AF_INET6, ip.family());
  EXPECT_EQ(kIPv6AnyString, ip.ToString());
}

TEST(IPAddressTest, TestUint32Constructor) {
  IPAddress ip(0);
  EXPECT_TRUE(IPIsAny(ip));
  EXPECT_FALSE(IPIsLoopback(ip));
  EXPECT_EQ(AF_INET, ip.family());
  EXPECT_EQ(kIPv4AnyString, ip.ToString());

  ip = IPAddress(INADDR_LOOPBACK);
  EXPECT_TRUE(IPIsLoopback(ip));
  EXPECT_FALSE(IPIsAny(ip));
  EXPECT_EQ(AF_INET, ip.family());
  EXPECT_EQ(kIPv4LoopbackString, ip.ToString());

  ip = IPAddress(INADDR_ANY);
  EXPECT_TRUE(IPIsAny(ip));
  EXPECT_FALSE(IPIsLoopback(ip));
  EXPECT_EQ(AF_INET, ip.family());
  EXPECT_EQ(kIPv4AnyString, ip.ToString());
}

TEST(IPAddressTest, TestCopyConstructor) {
  in_addr ipv4;
  ipv4.s_addr = kIPv4PublicAddr;
  IPAddress ip1(ipv4);
  IPAddress ip2(ip1);
  EXPECT_TRUE(IPAddressEq(ip1, ip2));

  ip1 = IPAddress(INADDR_LOOPBACK);
  ip2 = IPAddress(ip1);
  EXPECT_TRUE(IPAddressEq(ip1, ip2));

  ip1 = IPAddress(INADDR_ANY);
  ip2 = IPAddress(ip1);
  EXPECT_TRUE(IPAddressEq(ip1, ip2));

  ip1 = IPAddress(kIPv4PublicAddr);
  ip2 = IPAddress(ip1);
  EXPECT_TRUE(IPAddressEq(ip1, ip2));

  ip1 = IPAddress(kIPv6PublicAddr);
  ip2 = IPAddress(ip1);
  EXPECT_TRUE(IPAddressEq(ip1, ip2));
}

TEST(IPAddressTest, TestEquality) {
  // check ipv4 equality
  in_addr ipv4, ipv4_2;
  ipv4.s_addr = kIPv4PublicAddr;
  ipv4_2.s_addr = kIPv4PublicAddr + 1;
  IPAddress ip1(ipv4);
  IPAddress ip2(ipv4);
  IPAddress ip3(ipv4_2);
  EXPECT_TRUE(ip1 == ip1);
  EXPECT_TRUE(ip2 == ip2);
  EXPECT_TRUE(ip3 == ip3);
  EXPECT_TRUE(ip1 == ip2);
  EXPECT_TRUE(ip2 == ip1);
  EXPECT_FALSE(ip1 == ip3);
  EXPECT_FALSE(ip3 == ip1);
  EXPECT_FALSE(ip2 == ip3);
  EXPECT_FALSE(ip3 == ip2);

  // check ipv6 equality
  IPAddress ip4(kIPv6PublicAddr);
  IPAddress ip5(kIPv6PublicAddr);
  IPAddress ip6(kIPv6PublicAddr2);
  EXPECT_TRUE(ip4 == ip4);
  EXPECT_TRUE(ip5 == ip5);
  EXPECT_TRUE(ip6 == ip6);
  EXPECT_TRUE(ip4 == ip5);
  EXPECT_TRUE(ip5 == ip4);
  EXPECT_FALSE(ip4 == ip6);
  EXPECT_FALSE(ip6 == ip4);
  EXPECT_FALSE(ip5 == ip6);
  EXPECT_FALSE(ip6 == ip5);

  // check ipv4/ipv6 equality
  EXPECT_FALSE(ip1 == ip4);
  EXPECT_FALSE(ip4 == ip1);
  EXPECT_FALSE(ip2 == ip4);
  EXPECT_FALSE(ip4 == ip2);
  EXPECT_FALSE(ip3 == ip4);
  EXPECT_FALSE(ip4 == ip3);
  EXPECT_FALSE(ip1 == ip5);
  EXPECT_FALSE(ip5 == ip1);
  EXPECT_FALSE(ip2 == ip5);
  EXPECT_FALSE(ip5 == ip2);
  EXPECT_FALSE(ip3 == ip5);
  EXPECT_FALSE(ip5 == ip3);
  EXPECT_FALSE(ip1 == ip6);
  EXPECT_FALSE(ip6 == ip1);
  EXPECT_FALSE(ip2 == ip6);
  EXPECT_FALSE(ip6 == ip2);
  EXPECT_FALSE(ip3 == ip6);
  EXPECT_FALSE(ip6 == ip3);

  // loopback and any
  IPAddress v4_loopback(INADDR_LOOPBACK);
  IPAddress v6_loopback(in6addr_loopback);
  EXPECT_FALSE(v4_loopback == v6_loopback);

  IPAddress v4_any(INADDR_ANY);
  IPAddress v6_any(in6addr_any);
  EXPECT_FALSE(v4_any == v6_any);
}

TEST(IPAddressTest, TestFromAddressInfo) {
  struct sockaddr_in sockaddr_v4;
  struct addrinfo addrinfo;
  memset(&sockaddr_v4, 0, sizeof(sockaddr_v4));
  // check we and get a valid IPv4 address.
  addrinfo.ai_addr = reinterpret_cast<struct sockaddr*>(&sockaddr_v4);
  sockaddr_v4.sin_family = AF_INET;
  sockaddr_v4.sin_addr.s_addr = htobe32(kIPv4PublicAddr);
  IPAddress ip;
  EXPECT_TRUE(IPFromAddressInfo(&addrinfo, &ip));

  IPAddress expected(kIPv4PublicAddr);
  EXPECT_EQ(ip, expected);

  // check we can get a valid IPv6 address.
  struct sockaddr_in6 sockaddr_v6;
  sockaddr_v6.sin6_family = AF_INET6;
  sockaddr_v6.sin6_addr = kIPv6PublicAddr;
  addrinfo.ai_addr = reinterpret_cast<struct sockaddr*>(&sockaddr_v6);
  EXPECT_TRUE(IPFromAddressInfo(&addrinfo, &ip));
  expected = IPAddress(kIPv6PublicAddr);
  EXPECT_EQ(ip, expected);

  // UNSPEC fails
  sockaddr_v6.sin6_family = AF_UNSPEC;
  EXPECT_FALSE(IPFromAddressInfo(&addrinfo, &ip));

  // zero data never crash
  memset(&addrinfo, 0, sizeof(addrinfo));
  EXPECT_FALSE(IPFromAddressInfo(&addrinfo, &ip));
}

TEST(IPAddressTest, TestFromString) {
  IPAddress ip;
  IPAddress ip2;

  ip2 = IPAddress(INADDR_LOOPBACK);
  EXPECT_TRUE(IPFromString(kIPv4LoopbackString, &ip));
  EXPECT_EQ(ip.ToString(), kIPv4LoopbackString);
  EXPECT_TRUE(IPAddressEq(ip, ip2));

  ip2 = IPAddress(INADDR_ANY);
  EXPECT_TRUE(IPFromString(kIPv4AnyString, &ip));
  EXPECT_EQ(ip.ToString(), kIPv4AnyString);
  EXPECT_TRUE(IPAddressEq(ip, ip2));

  ip2 = IPAddress(kIPv4PublicAddr);
  EXPECT_TRUE(IPFromString(kIPv4PublicAddrString, &ip));
  EXPECT_EQ(ip.ToString(), kIPv4PublicAddrString);
  EXPECT_TRUE(IPAddressEq(ip, ip2));

  ip2 = IPAddress(in6addr_loopback);
  EXPECT_TRUE(IPFromString(kIPv6LoopbackString, &ip));
  EXPECT_EQ(ip.ToString(), kIPv6LoopbackString);
  EXPECT_TRUE(IPAddressEq(ip, ip2));

  ip2 = IPAddress(in6addr_any);
  EXPECT_TRUE(IPFromString(kIPv6AnyString, &ip));
  EXPECT_EQ(ip.ToString(), kIPv6AnyString);
  EXPECT_TRUE(IPAddressEq(ip, ip2));

  ip2 = IPAddress(kIPv6PublicAddr);
  EXPECT_TRUE(IPFromString(kIPv6PublicAddrString, &ip));
  EXPECT_EQ(ip.ToString(), kIPv6PublicAddrString);
  EXPECT_TRUE(IPAddressEq(ip, ip2));
}

}  // namespace net
}  // namespace base
}  // namespace avp
