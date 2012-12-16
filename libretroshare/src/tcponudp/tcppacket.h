/*
 * "$Id: tcppacket.h,v 1.3 2007-02-18 21:46:50 rmf24 Exp $"
 *
 * TCP-on-UDP (tou) network interface for RetroShare.
 *
 * Copyright 2004-2006 by Robert Fernie.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "retroshare@lunamutt.com".
 *
 */



#ifndef TOU_TCP_PACKET_H
#define TOU_TCP_PACKET_H

#include <sys/types.h>


typedef unsigned int   uint32;
typedef unsigned short uint16;
typedef unsigned char  uint8;

#define TCP_PSEUDO_HDR_SIZE 20

class TcpPacket
{
	public:

	uint8 *data;
	int   datasize;


	/* ports aren't needed -> in udp 
	 * uint16 srcport, destport
	 **************************/
	uint32 seqno, ackno;
	uint16 hlen_flags;
	uint16 winsize;
	/* don't need these -> in udp + not supported
	uint16 chksum, urgptr; 
	 **************************/
	/* no options.
	 **************************/
	

	/* other variables */
	double  ts; /* transmit time */ 
	uint16  retrans; /* retransmit counter */

	TcpPacket(uint8 *ptr, int size);
	TcpPacket(); /* likely control packet */
	~TcpPacket();

int	writePacket(void *buf, int &size);
int	readPacket(void *buf, int size);

void    *getData();
void    *releaseData();

void    *setData(void *data, int size);
int 	getDataSize();

	/* flags */
bool	hasSyn();
bool 	hasFin();
bool	hasAck();
bool	hasRst();

void    setSyn();
void    setFin();
void    setRst();
void    setAckFlag();

void    setAck(uint32 val);
uint32  getAck();


};


#endif

