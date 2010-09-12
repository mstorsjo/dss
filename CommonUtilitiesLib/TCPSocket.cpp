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
    File:       TCPSocket.cpp

    Contains:   implements TCPSocket class
                    
    
    
*/

#ifndef __Win32__
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#include <errno.h>

#include "TCPSocket.h"
#include "SocketUtils.h"
#include "OS.h"

#ifdef USE_NETLOG
#include <netlog.h>
#endif

void TCPSocket::SnarfSocket( TCPSocket & fromSocket )
{
    // take the connection away from the other socket
    // and use it as our own.
    Assert(fFileDesc == EventContext::kInvalidFileDesc);
    this->Set( fromSocket.fFileDesc, fromSocket.fRemoteAddr );
    
    // clear the old socket so he doesn't close and the like
    fromSocket.Set( EventContext::kInvalidFileDesc, Address() );

    // get the event context too
    this->SnarfEventContext( fromSocket );

}

void TCPSocket::Set(int inSocket, Address remoteaddr)
{
    fRemoteAddr = remoteaddr;
    fFileDesc = inSocket;
    
    if ( inSocket != EventContext::kInvalidFileDesc ) 
    {
        //make sure to find out what IP address this connection is actually occuring on. That
        //way, we can report correct information to clients asking what the connection's IP is
        sockaddr_storage ss;
#if __Win32__ || __osf__ || __sgi__ || __hpux__	
        int len = sizeof(ss);
#else
        socklen_t len = sizeof(ss);
#endif
        memset(&ss, 0, sizeof(ss));
        int err = ::getsockname(fFileDesc, (struct sockaddr*)&ss, &len);
        fLocalAddr = Address((struct sockaddr*) &ss);
        AssertV(err == 0, OSThread::GetErrno());
        fState |= kBound;
        fState |= kConnected;
    }
    else
        fState = 0;
}

StrPtrLen*  TCPSocket::GetRemoteAddrStr()
{
    if (fRemoteStr.Len == kIPAddrBufSize)
        fRemoteAddr.GetNumericString(&fRemoteStr);
    return &fRemoteStr;
}

OS_Error  TCPSocket::Connect(Address inRemoteAddr, UInt16 inRemotePort)
{
    fRemoteAddr = inRemoteAddr;
    fRemoteAddr.SetPort(inRemotePort);

    /* don't forget to error check the connect()! */
    int err = ::connect(fFileDesc, fRemoteAddr.GetSockAddr(), fRemoteAddr.GetSockLen());
    fState |= kConnected;
    
    if (err == -1)
    {
        fRemoteAddr = Address();
        return (OS_Error)OSThread::GetErrno();
    }
    
    return OS_NoErr;

}

