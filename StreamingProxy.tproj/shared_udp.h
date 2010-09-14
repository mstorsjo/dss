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
    File:       shared_udp.h
    Contains:   udp sockets implementation with shared ports

*/


#ifndef __SHARED_UDP_H__
#define __SHARED_UDP_H__

#include <sys/socket.h>

#define MAX_SOCKET_NAME 32

/**********************************************/
typedef int (*do_routine)(void * refCon, char *buf, int bufSize);

typedef struct ipList {
    struct ipList   *next;
    struct sockaddr_storage     ip;
    do_routine  what_to_do;
    void        *what_to_do_it_with;
} ipList;

typedef struct shok {
    struct shok *next;
    int     socket;
    int     port;
    struct sockaddr_storage fromIP;
    ipList      *ips;
    struct shok *sib;       // sibling - rtcp or rtp
} shok;

typedef struct trans_pb {
    int     *status;    // set to 1 when needs to die
    shok        *send_from;
    struct sockaddr_storage     send_to_ip;
    int     send_to_port;
    long long int   packetSendCount;
    long long int   nextDropPacket;
    long long int   droppedPacketCount;
    long long int   packetCount;
    char        socketName[MAX_SOCKET_NAME];

} trans_pb;

/**********************************************/
ipList *find_ip_in_list(ipList *list, struct sockaddr_storage ip);
int add_ip_to_list(ipList **list, struct sockaddr_storage ip);
int remove_ip_from_list(ipList **list, struct sockaddr_storage ip);
shok *find_available_shok(struct sockaddr_storage fromIP, struct sockaddr_storage toIP, int withSib);
int add_ips_to_shok(shok *theShok, struct sockaddr_storage fromIP, struct sockaddr_storage toIP, int withSib);
void set_udp_port_min_and_max(int min, int max);
int remove_shok(shok *theShok, int withSib);
void remove_shok_ref(shok *theShok, struct sockaddr_storage fromIP, struct sockaddr_storage toIP, int withSib);
shok *make_new_shok(struct sockaddr_storage fromIP, struct sockaddr_storage toIP, int withSib);
int make_udp_port_pair(struct sockaddr_storage fromIP, struct sockaddr_storage toIP, shok **rtpSocket, shok **rtcpSocket);
int upon_receipt_from(shok *theShok, struct sockaddr_storage fromIP, do_routine doThis, void *withThis);
int service_shoks();
int transfer_data(void *refCon, char *buf, int bufSize);

#endif // __SHARED_UDP_H__

