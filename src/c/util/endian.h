/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2013, Regents of the University of California
 *
 * BSD license, See the LICENSE file for more information
 *
 * Author: Junxiao Shi <shijunxiao@email.arizona.edu>
 */

#ifndef NDN_ENDIAN_H
#define NDN_ENDIAN_H

#if defined(__APPLE__)

#include <libkern/OSByteOrder.h>
#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)
#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)
#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#elif defined(_WIN32)

#include <WinSock2.h>
#include <stdint.h>
#include <ndn-cpp/ndn-cpp-config.h>
#define htobe16(x) htons(x)
#define be16toh(x) ntohs(x)
#define htobe32(x) htonl(x)
#define be32toh(x) ntohl(x)
#if NDN_CPP_HAVE_HTONLL
// Windows 8 has htonll. Assume we have ntohll if we have htonll.
#define htobe64(x) htonll(x)
#define be64toh(x) ntohll(x)
#else
// Define directly.

#ifdef __cplusplus
extern "C" {
#endif

// htons(1) == 1 is true for big endian systems.
static __inline uint64_t htobe64(uint64_t x) { return htons(1) == 1 ? x : (((uint64_t)htonl(x)) << 32) + htonl(x >> 32); }
static __inline uint64_t be64toh(uint64_t x) { return htons(1) == 1 ? x : (((uint64_t)ntohl(x)) << 32) + ntohl(x >> 32); }

#ifdef  __cplusplus
}
#endif

#endif
//#define htole16(x)
//#define le16toh(x)
//#define htole32(x)
//#define le32toh(x)
//#define htole64(x)
//#define le64toh(x)

#elif defined(__FreeBSD__)
#include <sys/endian.h>
#else
// Linux, Cygwin, etc.
#include <endian.h>
#endif

#endif
