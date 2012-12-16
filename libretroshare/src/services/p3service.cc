/*
 * libretroshare/src/services p3service.cc
 *
 * 3P/PQI network interface for RetroShare.
 *
 * Copyright 2004-2008 by Robert Fernie.
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

#include "pqi/pqi.h"
#include "util/rsstring.h"
#include "services/p3service.h"
#include <iomanip>

/*****
 * #define SERV_DEBUG 1
 ****/

void    p3Service::addSerialType(RsSerialType *st)
{
	rsSerialiser->addSerialType(st);
}

RsItem *p3Service::recvItem()
{
	srvMtx.lock();   /*****   LOCK MUTEX *****/

	if (recv_queue.size() == 0)
	{
		srvMtx.unlock(); /***** UNLOCK MUTEX *****/
		return NULL; /* nothing there! */
	}

	/* get something off front */
	RsItem *item = recv_queue.front();
	recv_queue.pop_front();

	srvMtx.unlock(); /***** UNLOCK MUTEX *****/
	return item;
}


bool    p3Service::receivedItems()
{
	srvMtx.lock();   /*****   LOCK MUTEX *****/

	bool moreData = (recv_queue.size() != 0);

	srvMtx.unlock(); /***** UNLOCK MUTEX *****/

	return moreData;
}


int p3Service::sendItem(RsItem *item)
{
	srvMtx.lock();   /*****   LOCK MUTEX *****/

	send_queue.push_back(item);

	srvMtx.unlock(); /***** UNLOCK MUTEX *****/

	return 1;
}

	// overloaded pqiService interface.
int	p3Service::receive(RsRawItem *raw)
{
	srvMtx.lock();   /*****   LOCK MUTEX *****/

#ifdef SERV_DEBUG 
	std::cerr << "p3Service::receive()";
	std::cerr << std::endl;
#endif

	/* convert to RsServiceItem */
	uint32_t size = raw->getRawLength();
	RsItem *item = rsSerialiser->deserialise(raw->getRawData(), &size);
	if ((!item) || (size != raw->getRawLength()))
	{
		/* error in conversion */
#ifdef SERV_DEBUG 
		std::cerr << "p3Service::receive() Error" << std::endl;
		std::cerr << "p3Service::receive() Size: " << size << std::endl;
		std::cerr << "p3Service::receive() RawLength: " << raw->getRawLength() << std::endl;
#endif

		if (item)
		{
#ifdef SERV_DEBUG 
			std::cerr << "p3Service::receive() Bad Item:";
			std::cerr << std::endl;
			item->print(std::cerr, 0);
			std::cerr << std::endl;
#endif
			delete item;
			item=NULL ;
		}
	}


	/* if we have something - pass it on */
	if (item)
	{
#ifdef SERV_DEBUG 
		std::cerr << "p3Service::receive() item:";
		std::cerr << std::endl;
		item->print(std::cerr, 0);
		std::cerr << std::endl;
#endif

		/* ensure PeerId is transferred */
		item->PeerId(raw->PeerId());
		recv_queue.push_back(item);
	}

	/* cleanup input */
	delete raw;

	srvMtx.unlock(); /***** UNLOCK MUTEX *****/

	return (item != NULL);
}

RsRawItem *p3Service::send()
{
	srvMtx.lock();   /*****   LOCK MUTEX *****/

	if (send_queue.size() == 0)
	{
		srvMtx.unlock(); /***** UNLOCK MUTEX *****/
		return NULL; /* nothing there! */
	}

	/* get something off front */
	RsItem *si = send_queue.front();
	send_queue.pop_front();

#ifdef SERV_DEBUG 
	std::cerr << "p3Service::send() Sending item:";
	std::cerr << std::endl;
	si->print(std::cerr, 0);
	std::cerr << std::endl;
#endif

	/* try to convert */
	uint32_t size = rsSerialiser->size(si);
	if (!size)
	{
#ifdef SERV_DEBUG 
		std::cerr << "p3Service::send() ERROR size == 0";
		std::cerr << std::endl;
#endif

		/* can't convert! */
		delete si;
		srvMtx.unlock(); /***** UNLOCK MUTEX *****/
		return NULL;
	}

	RsRawItem *raw = new RsRawItem(si->PacketId(), size);
	if (!rsSerialiser->serialise(si, raw->getRawData(), &size))
	{
#ifdef SERV_DEBUG 
		std::cerr << "p3Service::send() ERROR serialise failed";
		std::cerr << std::endl;
#endif
		delete raw;
		raw = NULL;
	}

	if ((raw) && (size != raw->getRawLength()))
	{
#ifdef SERV_DEBUG 
		std::cerr << "p3Service::send() ERROR serialise size mismatch";
		std::cerr << std::endl;
#endif
		delete raw;
		raw = NULL;
	}

	/* ensure PeerId is transferred */
	if (raw)
	{
		raw->PeerId(si->PeerId());

		if(si->priority_level() == QOS_PRIORITY_UNKNOWN)
		{
			std::cerr << "************************************************************" << std::endl;
			std::cerr << "********** Warning: p3service::send()               ********" << std::endl;
			std::cerr << "********** Warning: caught a RsItem with undefined  ********" << std::endl;
			std::cerr << "**********          priority level. That should not ********" << std::endl;
			std::cerr << "**********          happen. Please fix your items!  ********" << std::endl;
			std::cerr << "************************************************************" << std::endl;
		}
		raw->setPriorityLevel(si->priority_level()) ;
	}

	/* cleanup */
	delete si;

	srvMtx.unlock(); /***** UNLOCK MUTEX *****/
#ifdef SERV_DEBUG
                std::cerr << "p3Service::send() returning RawItem.";
                std::cerr << std::endl;
#endif
	return raw;
}


std::string generateRandomServiceId()
{
	std::string out;

/********************************** WINDOWS/UNIX SPECIFIC PART ******************/
#ifndef WINDOWS_SYS
	/* 4 bytes per random number: 4 x 4 = 16 bytes */
	for(int i = 0; i < 4; i++)
	{
		uint32_t rint = random();
		rs_sprintf_append(out, "%08x", rint);
	}
#else
	srand(time(NULL));
	/* 2 bytes per random number: 8 x 2 = 16 bytes */
	for(int i = 0; i < 8; i++)
	{
		uint16_t rint = rand(); /* only gives 16 bits */
		rs_sprintf_append(out, "%04x", rint);
	}
#endif
/********************************** WINDOWS/UNIX SPECIFIC PART ******************/
	return out;
}
	

