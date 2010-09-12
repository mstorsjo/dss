/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 *
 */

#include "Address.h"


Address Address::ConvertStringToAddress(const char *s, bool resolve) {
    struct addrinfo hints, *ai, *ai0 = NULL;
    int err;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    if (!resolve)
        hints.ai_flags = AI_NUMERICHOST;

    ai0 = NULL;
    err = getaddrinfo(s, NULL, &hints, &ai0);
    if (err) {
        if (ai0) freeaddrinfo(ai0);
        ai0 = NULL;
    }

    for (ai = ai0; ai; ai = ai->ai_next) {
        Address addr(ai->ai_addr);
        if (ai0) freeaddrinfo(ai0);
        return addr;
    }

    if (ai0) freeaddrinfo(ai0);

    return CreateEmptyAddress(AF_INET);
}


Address Address::CreateAnyAddress(UInt32 index) {
    struct addrinfo hints, *ai, *ai0 = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(NULL, "0", &hints, &ai0);
    UInt32 count = 0;
    ai = ai0;
    while (ai) {
        if (index == count) {
            Address result(ai->ai_addr);
            freeaddrinfo(ai0);
            return result;
        }
        count++;
        ai = ai->ai_next;
    }
    freeaddrinfo(ai0);
    return Address();
}

UInt32 Address::CountAnyAddresses() {
    struct addrinfo hints, *ai, *ai0 = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(NULL, "0", &hints, &ai0);
    UInt32 count = 0;
    ai = ai0;
    while (ai) {
        count++;
        ai = ai->ai_next;
    }
    freeaddrinfo(ai0);
    return count;
}

Bool16 Address::IsFamilySupported(int family) {
    struct addrinfo hints, *ai, *ai0 = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(NULL, "0", &hints, &ai0);
    ai = ai0;
    bool supported = false;
    while (ai && !supported) {
        if (ai->ai_addr->sa_family == family)
            supported = true;
        ai = ai->ai_next;
    }
    freeaddrinfo(ai0);
    return supported;
}

Address Address::CreateAnyAddressOfFamily(int family) {
    struct addrinfo hints, *ai = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(NULL, "0", &hints, &ai);
    if (ai) {
        Address result(ai->ai_addr);
        freeaddrinfo(ai);
        return result;
    }
    freeaddrinfo(ai);
    return Address();
}

Address Address::CreateEmptyAddress(UInt32 sa_family) {
    switch (sa_family) {
    case AF_INET: {
        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        return Address((struct sockaddr*) &sin);
    }
    case AF_INET6: {
        struct sockaddr_in6 sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin6_family = AF_INET6;
        return Address((struct sockaddr*) &sin);
    }
    }
    return Address();
}

Address::Address() {
    init(NULL, 0);
}

Address::Address(const struct sockaddr* sa) {
    if (!sa) {
        init(NULL, 0);
        return;
    }
    switch (sa->sa_family) {
    case AF_INET:
        init(sa, sizeof(sockaddr_in));
        break;
    case AF_INET6:
        init(sa, sizeof(sockaddr_in6));
        break;
    }
}

void Address::init(const struct sockaddr *sa, socklen_t len) {
    memset(&m_sockaddr, 0, sizeof(m_sockaddr));
    if (sa) {
        m_family = sa->sa_family;
        m_socklen = OS::Min(len, sizeof(m_sockaddr));
        memcpy(&m_sockaddr, sa, m_socklen);
    } else {
        m_family = 0;
        m_socklen = 0;
    }

    m_ttl = 0;
}

int Address::GetFamily() const {
    return m_family;
}

UInt16 Address::GetPort() const {
    switch (m_family) {
    case AF_INET:
        return ntohs(((struct sockaddr_in*)&m_sockaddr)->sin_port);
    case AF_INET6:
        return ntohs(((struct sockaddr_in6*)&m_sockaddr)->sin6_port);
    }
    return 0;
}

void Address::SetPort(UInt16 port) {
    switch (m_family) {
    case AF_INET:
        ((struct sockaddr_in*)&m_sockaddr)->sin_port = htons(port);
        break;
    case AF_INET6:
        ((struct sockaddr_in6*)&m_sockaddr)->sin6_port = htons(port);
        break;
    }
}

Bool16 Address::IsEqual(const Address& address) const {
    if (m_family != address.m_family)
        return false;
    switch (m_family) {
    case AF_INET:
        return memcmp(&((struct sockaddr_in*)&m_sockaddr)->sin_addr,
                      &((struct sockaddr_in*)&address.m_sockaddr)->sin_addr,
                      sizeof(struct in_addr)) == 0;
    case AF_INET6:
        return memcmp(&((struct sockaddr_in6*)&m_sockaddr)->sin6_addr,
                      &((struct sockaddr_in6*)&address.m_sockaddr)->sin6_addr,
                      sizeof(struct in6_addr)) == 0;
    }
    return true;
}

socklen_t Address::GetSockLen() const {
    return m_socklen;
}

const struct sockaddr* Address::GetSockAddr() const {
    return (const struct sockaddr*)&m_sockaddr;
}

void Address::GetDNSString(StrPtrLen* outAddr) {
    if (getnameinfo(GetSockAddr(), m_socklen, outAddr->Ptr, outAddr->Len, NULL, 0, 0)) {
        perror("getnameinfo");
        ToNumericString(outAddr->Ptr, outAddr->Len);
    }
    outAddr->Ptr[outAddr->Len - 1] = '\0';
    outAddr->Len = strlen(outAddr->Ptr);
}

char* Address::ToNumericString(char* buffer, int length, bool urlQuoted) {
    char* ptr = buffer;
    if (urlQuoted && m_family == AF_INET6) {
        ptr++;
        length -= 3;
    }
    if (getnameinfo(GetSockAddr(), m_socklen, ptr, length, NULL, 0, NI_NUMERICHOST)) {
        perror("getnameinfo");
    }
    if (urlQuoted && m_family == AF_INET6) {
        buffer[0] = '[';
        length = strlen(buffer);
        buffer[length] = ']';
        buffer[length + 1] = '\0';
    } else {
        buffer[length - 1] = '\0';
    }
    return buffer;
}

Bool16 Address::IsAddrLoopback() const {
    switch (m_family) {
    case AF_INET:
        return ((const struct sockaddr_in*)&m_sockaddr)->sin_addr.s_addr == htonl(INADDR_LOOPBACK);
    case AF_INET6:
        return IN6_IS_ADDR_LOOPBACK(&((const struct sockaddr_in6*)&m_sockaddr)->sin6_addr);
    }
    return false;
}

Bool16 Address::IsAddrAny() const {
    switch (m_family) {
    case AF_INET:
        return ((const struct sockaddr_in*)&m_sockaddr)->sin_addr.s_addr == htonl(INADDR_ANY);
    case AF_INET6:
        return IN6_IS_ADDR_UNSPECIFIED(&((const struct sockaddr_in6*)&m_sockaddr)->sin6_addr);
    }
    return false;
}

Bool16 Address::IsAddrEmpty() const {
    if (m_family == 0)
        return true;
    return IsAddrAny();
}

Bool16 Address::IsMulticast() const {
    switch (m_family) {
    case AF_INET:
        return IN_MULTICAST(ntohl(((const struct sockaddr_in*)&m_sockaddr)->sin_addr.s_addr));
    case AF_INET6:
        return IN6_IS_ADDR_MULTICAST(&((const struct sockaddr_in6*)&m_sockaddr)->sin6_addr);
    }
    return false;
}

UInt32 Address::GetHashValue() const {
    switch (m_family) {
    case AF_INET:
        return ((const struct sockaddr_in*)&m_sockaddr)->sin_addr.s_addr;
    case AF_INET6:
        return *(UInt32*)&((const struct sockaddr_in6*)&m_sockaddr)->sin6_addr.s6_addr[12];
    }
    return 0;
}

int Address::JoinMulticast(int fd, Address remote) {
    struct ip_mreq mreq;
    struct ipv6_mreq mreq6;
    switch (m_family) {
    case AF_INET:
        mreq.imr_multiaddr.s_addr = ((struct sockaddr_in*)&remote.m_sockaddr)->sin_addr.s_addr;
        mreq.imr_interface.s_addr = ((struct sockaddr_in*)&m_sockaddr)->sin_addr.s_addr;
        return setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    case AF_INET6:
        memcpy(&mreq6.ipv6mr_multiaddr, &((struct sockaddr_in6*)&remote.m_sockaddr)->sin6_addr, sizeof(struct in6_addr));
        mreq6.ipv6mr_interface = 0;
        return setsockopt(fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq6, sizeof(mreq6));
    }
    return -1;
}

int Address::LeaveMulticast(int fd, Address remote) {
    struct ip_mreq mreq;
    struct ipv6_mreq mreq6;
    switch (m_family) {
    case AF_INET:
        mreq.imr_multiaddr.s_addr = ((struct sockaddr_in*)&remote.m_sockaddr)->sin_addr.s_addr;
        mreq.imr_interface.s_addr = ((struct sockaddr_in*)&m_sockaddr)->sin_addr.s_addr;
        return setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
    case AF_INET6:
        memcpy(&mreq6.ipv6mr_multiaddr, &((struct sockaddr_in6*)&remote.m_sockaddr)->sin6_addr, sizeof(struct in6_addr));
        mreq6.ipv6mr_interface = 0;
        return setsockopt(fd, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &mreq6, sizeof(mreq6));
    }
    return -1;
}

int Address::SetTTL(UInt16 timeToLive) {
    m_ttl = timeToLive;
    return 0;
}

int Address::SetTTL(int fd, UInt16 timeToLive) {
    int mttlint = timeToLive;
    m_ttl = timeToLive;
    switch (m_family) {
    case AF_INET:
        return setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &mttlint, sizeof(mttlint));
    case AF_INET6:
        return setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &mttlint, sizeof(mttlint));
    }
    return -1;
}

int Address::SetMulticastInterface(int fd) {
    switch (m_family) {
    case AF_INET:
        return setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &((struct sockaddr_in*)&m_sockaddr)->sin_addr, sizeof(struct in_addr));
    case AF_INET6:
        return setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &((struct sockaddr_in6*)&m_sockaddr)->sin6_addr, sizeof(struct in6_addr));
    }
    return -1;
}

void Address::SetLoopback(int family) {
    if (family)
        m_family = family;
    switch (m_family) {
    case AF_INET:
        ((struct sockaddr_in*)&m_sockaddr)->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        break;
    case AF_INET6:
        ((struct sockaddr_in6*)&m_sockaddr)->sin6_addr = in6addr_loopback;
        break;
    }
}

void Address::GetNumericString(StrPtrLen* outAddr) {
    ToNumericString(outAddr->Ptr, outAddr->Len);
    outAddr->Ptr[outAddr->Len - 1] = '\0';
    outAddr->Len = strlen(outAddr->Ptr);
}

char* Address::GetSDPCLine(char* buffer) {
    char addrbuf[ADDRSTRLEN];
    switch (m_family) {
    case AF_INET:
        if (IsMulticast()) qtss_sprintf(buffer, "c=IN IP4 %s/%d", ToNumericString(addrbuf), m_ttl);
        else qtss_sprintf(buffer, "c=IN IP4 %s", ToNumericString(addrbuf));
        break;
    case AF_INET6:
        if (IsMulticast()) qtss_sprintf(buffer, "c=IN IP6 %s/%d", ToNumericString(addrbuf), m_ttl);
        else qtss_sprintf(buffer, "c=IN IP6 %s", ToNumericString(addrbuf));
        break;
    default:
        buffer[0] = '\0';
        break;
    }
    return buffer;
}

char* Address::GetSDPINLine(char* buffer) {
    char addrbuf[ADDRSTRLEN];
    switch (m_family) {
    case AF_INET:
        if (IsMulticast()) qtss_sprintf(buffer, "IN IP4 %s/%d", ToNumericString(addrbuf), m_ttl);
        else qtss_sprintf(buffer, "IN IP4 %s", ToNumericString(addrbuf));
        break;
    case AF_INET6:
        if (IsMulticast()) qtss_sprintf(buffer, "IN IP6 %s/%d", ToNumericString(addrbuf), m_ttl);
        else qtss_sprintf(buffer, "IN IP6 %s", ToNumericString(addrbuf));
        break;
    default:
        buffer[0] = '\0';
        break;
    }
    return buffer;
}

const char* Address::GetSDPINWord() {
    switch (m_family) {
    case AF_INET:
        return "IP4";
    case AF_INET6:
        return "IP6";
    }
    return "";
}

