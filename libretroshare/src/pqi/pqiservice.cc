/*
 * libretroshare/src/pqi pqiservice.cc
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

#include "pqi/pqiservice.h"
#include "util/rsdebug.h"
#include "util/rsstring.h"

const int pqiservicezone = 60478;

/****
 * #define SERVICE_DEBUG 1
 ****/

p3ServiceServer::p3ServiceServer() : srvMtx("p3ServiceServer")
{
	RsStackMutex stack(srvMtx); /********* LOCKED *********/

#ifdef  SERVICE_DEBUG
	pqioutput(PQL_DEBUG_BASIC, pqiservicezone, 
		"p3ServiceServer::p3ServiceServer()");
#endif

        rrit = services.begin();
	return;
}

int	p3ServiceServer::addService(pqiService *ts)
{
	RsStackMutex stack(srvMtx); /********* LOCKED *********/

#ifdef  SERVICE_DEBUG
	pqioutput(PQL_DEBUG_BASIC, pqiservicezone, 
		"p3ServiceServer::addService()");
#endif

	std::map<uint32_t, pqiService *>::iterator it;
	it = services.find(ts -> getType());
	if (it != services.end())
	{
		// it exists already!
		return -1;
	}

	services[ts -> getType()] = ts;
        rrit = services.begin();
	return 1;
}

int	p3ServiceServer::incoming(RsRawItem *item)
{
	RsStackMutex stack(srvMtx); /********* LOCKED *********/

#ifdef  SERVICE_DEBUG
	pqioutput(PQL_DEBUG_BASIC, pqiservicezone, 
		"p3ServiceServer::incoming()");

	{
		std::string out;
		rs_sprintf(out, "p3ServiceServer::incoming() PacketId: %x\nLooking for Service: %x\nItem:\n", item -> PacketId(), (item -> PacketId() & 0xffffff00));
		item -> print_string(out);
		pqioutput(PQL_DEBUG_BASIC, pqiservicezone, out);
	}
#endif

	std::map<uint32_t, pqiService *>::iterator it;
	it = services.find(item -> PacketId() & 0xffffff00);
	if (it == services.end())
	{
#ifdef  SERVICE_DEBUG
		pqioutput(PQL_DEBUG_BASIC, pqiservicezone, 
		"p3ServiceServer::incoming() Service: No Service - deleting");
#endif

		// delete it.
		delete item;

		// it exists already!
		return -1;
	}

	{
#ifdef  SERVICE_DEBUG
		std::string out;
		rs_sprintf(out, "p3ServiceServer::incoming() Sending to %p", it -> second);
		pqioutput(PQL_DEBUG_BASIC, pqiservicezone, out);
#endif

		return (it->second) -> receive(item);
	}

	delete item;
	return -1;
}



RsRawItem *p3ServiceServer::outgoing()
{
	RsStackMutex stack(srvMtx); /********* LOCKED *********/

#ifdef  SERVICE_DEBUG
	pqioutput(PQL_DEBUG_ALL, pqiservicezone, 
		"p3ServiceServer::outgoing()");
#endif

	if (rrit != services.end())
	{
		rrit++;
	}
	else
	{
		rrit = services.begin();
	}

	std::map<uint32_t, pqiService *>::iterator sit = rrit;
	// run to the end.
	RsRawItem *item;

	// run through to the end,
	for(;rrit != services.end();rrit++)
	{
		if (NULL != (item = (rrit -> second) -> send()))
		{

#ifdef  SERVICE_DEBUG
			std::string out;
			rs_sprintf(out, "p3ServiceServer::outgoing() Got Item From: %p\n", rrit -> second);

			item -> print_string(out);
			std::cerr << out << std::endl;
			pqioutput(PQL_DEBUG_BASIC, pqiservicezone, out);
#endif

			return item;
		}
	}

	// from the beginning to where we started.
	for(rrit = services.begin();rrit != sit; rrit++)
	{
		if (NULL != (item = (rrit -> second) -> send()))
		{

#ifdef  SERVICE_DEBUG
			std::string out;
			rs_sprintf(out, "p3ServiceServer::outgoing() Got Item From: %p\n", rrit -> second);

			item -> print_string(out);
			pqioutput(PQL_DEBUG_BASIC, pqiservicezone, out);
			std::cerr << out << std::endl;
#endif

			return item;
		}
	}
	return NULL;
}



int	p3ServiceServer::tick()
{

	RsStackMutex stack(srvMtx); /********* LOCKED *********/

#ifdef  SERVICE_DEBUG
	pqioutput(PQL_DEBUG_ALL, pqiservicezone, 
		"p3ServiceServer::tick()");
#endif

	std::map<uint32_t, pqiService *>::iterator it;

	// from the beginning to where we started.
	for(it = services.begin();it != services.end(); it++)
	{

#ifdef  SERVICE_DEBUG
		std::string out;
		rs_sprintf(out, "p3ServiceServer::service id: %u -> Service: %p", it -> first, it -> second);
		pqioutput(PQL_DEBUG_ALL, pqiservicezone, out);
#endif

		// now we should actually tick the service.
		(it -> second) -> tick();
	}
	return 1;
}



