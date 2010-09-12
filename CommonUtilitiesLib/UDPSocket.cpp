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
/*
    File:       UDPSocket.cpp

    Contains:   Implementation of object defined in UDPSocket.h.

    
    
*/

#ifndef __Win32__
#include <sys/types.h>
#include <sys/socket.h>

#if __solaris__
#include "SocketUtils.h"
#endif

#if NEED_SOCKETBITS
#if __GLIBC__ >= 2
#include <bits/socket.h>
#else
#include <socketbits.h>
#endif
#endif
#endif

#include <errno.h>
#include "UDPSocket.h"
#include "OSMemory.h"

#ifdef USE_NETLOG
#include <netlog.h>
#endif

UDPSocket::UDPSocket(Task* inTask, UInt32 inSocketType)
: Socket(inTask, inSocketType), fDemuxer(NULL)
{
    if (inSocketType & kWantsDemuxer)
        fDemuxer = NEW UDPDemuxer();
        
    //setup msghdr
    ::memset(&fMsgAddr, 0, sizeof(fMsgAddr));
}


OS_Error
UDPSocket::SendTo(Address inRemoteAddr, UInt16 inRemotePort, void* inBuffer, UInt32 inLength)
{
    Assert(inBuffer != NULL);
    
    Address theRemoteAddr = inRemoteAddr;
    theRemoteAddr.SetPort(inRemotePort);

#ifdef __sgi__
	int theErr = ::sendto(fFileDesc, inBuffer, inLength, 0, theRemoteAddr.GetSockAddr(), theRemoteAddr.GetSockLen());
#else
    // Win32 says that inBuffer is a char*
	int theErr = ::sendto(fFileDesc, (char*)inBuffer, inLength, 0, theRemoteAddr.GetSockAddr(), theRemoteAddr.GetSockLen());
#endif

    if (theErr == -1)
        return (OS_Error)OSThread::GetErrno();
    return OS_NoErr;
}

OS_Error UDPSocket::RecvFrom(Address* outRemoteAddr, UInt16* outRemotePort,
                            void* ioBuffer, UInt32 inBufLen, UInt32* outRecvLen)
{
    Assert(outRecvLen != NULL);
    Assert(outRemoteAddr != NULL);
    Assert(outRemotePort != NULL);
    
#if __Win32__ || __osf__  || __sgi__ || __hpux__
    int addrLen = sizeof(fMsgAddr);
#else
    socklen_t addrLen = sizeof(fMsgAddr);
#endif

#ifdef __sgi__
	SInt32 theRecvLen = ::recvfrom(fFileDesc, ioBuffer, inBufLen, 0, (sockaddr*)&fMsgAddr, &addrLen);
#else
    // Win32 says that ioBuffer is a char*
    SInt32 theRecvLen = ::recvfrom(fFileDesc, (char*)ioBuffer, inBufLen, 0, (sockaddr*)&fMsgAddr, &addrLen);
#endif

    if (theRecvLen == -1)
        return (OS_Error)OSThread::GetErrno();
    
    *outRemoteAddr = Address((struct sockaddr*) &fMsgAddr);
    *outRemotePort = outRemoteAddr->GetPort();
    Assert(theRecvLen >= 0);
    *outRecvLen = (UInt32)theRecvLen;
    return OS_NoErr;        
}

OS_Error UDPSocket::JoinMulticast(Address inRemoteAddr)
{
    int err = fLocalAddr.JoinMulticast(fFileDesc, inRemoteAddr);
    //AssertV(err == 0, OSThread::GetErrno());
    if (err == -1)
         return (OS_Error)OSThread::GetErrno();
    else
         return OS_NoErr;
}

OS_Error UDPSocket::SetTtl(UInt16 timeToLive)
{
    int err = fLocalAddr.SetTTL(fFileDesc, timeToLive);
    if (err == -1)
        return (OS_Error)OSThread::GetErrno();
    else
        return OS_NoErr;    
}

OS_Error UDPSocket::SetMulticastInterface(Address inLocalAddr)
{
    int err = inLocalAddr.SetMulticastInterface(fFileDesc);
    AssertV(err == 0, OSThread::GetErrno());
    if (err == -1)
        return (OS_Error)OSThread::GetErrno();
    else
        return OS_NoErr;    
}

OS_Error UDPSocket::LeaveMulticast(Address inRemoteAddr)
{
    int err = fLocalAddr.LeaveMulticast(fFileDesc, inRemoteAddr);
    if (err == -1)
        return (OS_Error)OSThread::GetErrno();
    else
        return OS_NoErr;    
}
