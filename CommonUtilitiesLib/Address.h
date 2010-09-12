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

#ifndef __ADDRESS_H__
#define __ADDRESS_H__

#ifndef __Win32__
#include <sys/socket.h>
#include <netdb.h>
#endif

#include "OS.h"

#define ADDRSTRLEN           NI_MAXHOST
#define SDPLINEBUFSIZE       (ADDRSTRLEN + 50)

#ifndef IPV6_ADD_MEMBERSHIP
#define IPV6_ADD_MEMBERSHIP  IPV6_JOIN_GROUP
#define IPV6_DROP_MEMBERSHIP IPV6_LEAVE_GROUP
#endif

class Address
{
    public:
        Address();
        Address(const sockaddr* sa);

        int         GetFamily() const;
        UInt16      GetPort() const;
        void        SetPort(UInt16);
        socklen_t   GetSockLen() const;
        const sockaddr* GetSockAddr() const;
        Bool16      IsEqual(const Address&) const;
        void        GetNumericString(StrPtrLen*);
        void        GetDNSString(StrPtrLen*);
        char*       ToNumericString(char* buffer, int length = ADDRSTRLEN, bool urlQuoted = false);
        char*       GetSDPCLine(char* buffer);
        char*       GetSDPINLine(char* buffer);
        const char* GetSDPINWord();
        Bool16      IsAddrAny() const;
        Bool16      IsAddrEmpty() const;
        Bool16      IsAddrLoopback() const;
        Bool16      IsMulticast() const;
        UInt32      GetHashValue() const;
        int         JoinMulticast(int, Address);
        int         LeaveMulticast(int, Address);
        int         SetTTL(UInt16);
        int         SetTTL(int, UInt16);
        int         SetMulticastInterface(int);
        void        SetLoopback(int family = 0);

        static Address      ConvertStringToAddress(const char*, bool resolve = true);
        static Address      CreateAnyAddress(UInt32 index = 0);
        static Address      CreateAnyAddressOfFamily(int family);
        static UInt32       CountAnyAddresses();
        static Address      CreateEmptyAddress(UInt32);
        static Bool16       IsFamilySupported(int family);

        bool operator==(const Address& a) const { return IsEqual(a); }
        bool operator!=(const Address& a) const { return !IsEqual(a); }

    protected:

        void init(const struct sockaddr*, socklen_t);

        int                 m_family;
        UInt16              m_ttl;
        socklen_t           m_socklen;
        sockaddr_storage    m_sockaddr;

};

#endif // __ADDRESS_H__
