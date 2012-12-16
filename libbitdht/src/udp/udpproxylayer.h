#ifndef BITDHT_UDP_LAYER_H
#define BITDHT_UDP_LAYER_H

/*
 * udp/udplayer.h
 *
 * BitDHT: An Flexible DHT library.
 * 
 * Copyright 2004-2010 by Robert Fernie
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 3 as published by the Free Software Foundation.
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
 * Please report all bugs and problems to "bitdht@lunamutt.com".
 *
 */

#include "util/bdthreads.h"
#include "util/bdnet.h"

#include <iosfwd>
#include <list>
#include <deque>

/* careful - duplicate definitions */
//std::ostream &operator<<(std::ostream &out,  const struct sockaddr_in &addr);
std::ostream &operator<<(std::ostream &out,  struct sockaddr_in &addr);

bool operator==(const struct sockaddr_in &addr, const struct sockaddr_in &addr2);
bool operator<(const struct sockaddr_in &addr, const struct sockaddr_in &addr2);

std::string printPkt(void *d, int size);
std::string printPktOffset(unsigned int offset, void *d, unsigned int size);


/* UdpLayer ..... is the bottom layer which 
 * just sends and receives Udp packets.
 */

class UdpReceiver
{
	public:
virtual ~UdpReceiver() {}
virtual int recvPkt(void *data, int size, struct sockaddr_in &from) = 0;
virtual int status(std::ostream &out) = 0;
};

class UdpPublisher
{
	public:
virtual ~UdpPublisher() {}
virtual	int sendPkt(const void *data, int size, const struct sockaddr_in &to, int ttl) = 0;
};


class UdpLayer: public bdThread
{
	public:

	UdpLayer(UdpReceiver *recv, struct sockaddr_in &local);
virtual ~UdpLayer() { return; }

int 	reset(struct sockaddr_in &local); /* calls join, close, openSocket */

int     status(std::ostream &out);

	/* setup connections */
	int closeSocket();
	int openSocket();

	/* RsThread functions */
virtual void run(); /* called once the thread is started */

void	recv_loop(); /* uses callback to UdpReceiver */

	/* Higher Level Interface */
	//int  readPkt(void *data, int *size, struct sockaddr_in &from);
	int  sendPkt(const void *data, int size, const struct sockaddr_in &to, int ttl);

	/* monitoring / updates */
	int okay();
	int tick();


	/* data */
	/* internals */
	protected:

virtual	int receiveUdpPacket(void *data, int *size, struct sockaddr_in &from);
virtual	int sendUdpPacket(const void *data, int size, const struct sockaddr_in &to);
 
	int setTTL(int t);
	int getTTL();

	/* low level */
	private:

	UdpReceiver *recv;

	struct sockaddr_in laddr; /* local addr */

	int  errorState;
	int sockfd;
	int ttl;
	bool stopThread;

	bdMutex sockMtx;
};


/* For Testing - drops packets */
class LossyUdpLayer: public UdpLayer
{
	public:
  LossyUdpLayer(UdpReceiver *udpr, struct sockaddr_in &local, double frac);
virtual ~LossyUdpLayer();

        protected:

virtual int receiveUdpPacket(void *data, int *size, struct sockaddr_in &from);
virtual	int sendUdpPacket(const void *data, int size, const struct sockaddr_in &to);

	double lossFraction;
};

class PortRange
{
	public:
	PortRange();
	PortRange(uint16_t lp, uint16_t up);

	bool inRange(uint16_t port);

	uint16_t lport;
	uint16_t uport;
};


/* For Testing - drops packets */
class RestrictedUdpLayer: public UdpLayer
{
	public:
  RestrictedUdpLayer(UdpReceiver *udpr, struct sockaddr_in &local);
virtual ~RestrictedUdpLayer();

void	addRestrictedPortRange(int lp, int up);

        protected:

virtual int receiveUdpPacket(void *data, int *size, struct sockaddr_in &from);
virtual	int sendUdpPacket(const void *data, int size, const struct sockaddr_in &to);

	std::list<PortRange> mLostPorts;
};


/* For Testing - drops packets all packets for initial minute (simulates TTL) */
class TimedUdpLayer: public UdpLayer
{
	public:
  TimedUdpLayer(UdpReceiver *udpr, struct sockaddr_in &local);
virtual ~TimedUdpLayer();

        protected:

virtual int receiveUdpPacket(void *data, int *size, struct sockaddr_in &from);
virtual	int sendUdpPacket(const void *data, int size, const struct sockaddr_in &to);

	time_t mStartTime;
	bool mActive;

};


#endif
