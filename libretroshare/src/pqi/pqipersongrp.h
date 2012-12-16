/*
 * libretroshare/src/pqi: pqipersongrp.h
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


#ifndef MRK_PQI_PERSON_HANDLER_HEADER
#define MRK_PQI_PERSON_HANDLER_HEADER

#include "pqi/pqihandler.h"
#include "pqi/pqiperson.h"
#include "pqi/pqilistener.h"
#include "pqi/pqiservice.h"
#include "pqi/pqimonitor.h"
#include "pqi/p3cfgmgr.h"


// So this is a specific implementation 
//
// it is designed to have one pqilistensocket + a series of pqisockets
//
// as an added bonus, we are going to 
// make this a pqitunnelserver, to which services can be attached.

const unsigned long PQIPERSON_NO_LISTENER = 	0x0001;

const unsigned long PQIPERSON_ALL_BW_LIMITED =  0x0010;

class pqipersongrp: public pqihandler, public pqiMonitor, public p3ServiceServer, public pqiNetListener
{
	public:
	pqipersongrp(SecurityPolicy *, unsigned long flags);

	/*************************** Setup *************************/
	/* pqilistener */

virtual bool resetListener(struct sockaddr_in &local); // overloaded from pqiNetListener 
int     init_listener(); 

	/*************** pqiMonitor callback ***********************/
virtual void    statusChange(const std::list<pqipeer> &plist);

#ifdef WINDOWS_SYS
///////////////////////////////////////////////////////////
// hack for too many connections
virtual void    statusChanged();
///////////////////////////////////////////////////////////
#endif

	/******************* Peer Control **************************/
virtual int addPeer(std::string id); /* can be overloaded for testing */
int     removePeer(std::string id);
int     connectPeer(std::string id
#ifdef WINDOWS_SYS
///////////////////////////////////////////////////////////
// hack for too many connections
					, bool bConnect = false
///////////////////////////////////////////////////////////
#endif
					);

	/* Work-around to dodgy pointer stuff */
int	tagHeartbeatRecvd(std::string id);

	/*** callback from children ****/
bool    notifyConnect(std::string id, uint32_t type, bool success, struct sockaddr_in remote_peer_address);
//bool    notifyConnect(std::string id, uint32_t type, bool success);

	// tick interfaces.
virtual int tick();
virtual int status();

	protected:

	/********* FUNCTIONS to OVERLOAD for specialisation ********/
	// THESE NEED TO BE LOCKED UNTIL PQILISTENER IS THREAD-SAFE.
virtual pqilistener *locked_createListener(struct sockaddr_in laddr) = 0;
virtual pqiperson   *locked_createPerson(std::string id, pqilistener *listener) = 0;
	/********* FUNCTIONS to OVERLOAD for specialisation ********/

	/* Overloaded RsItem Check
	 * checks item->cid vs Person
	 */
virtual int checkOutgoingRsItem(RsItem *item, int global) 
	{ 
                (void) item;   /* suppress unused parameter warning */
                (void) global; /* suppress unused parameter warning */
		return 1; 
	}

	private:

	// The tunnelserver operation.
	int tickServiceRecv();
	int tickServiceSend();

	pqilistener *pqil;
	unsigned long initFlags;
};

class pqipersongrpDummy: public pqipersongrp
{
	public:
	pqipersongrpDummy(SecurityPolicy *pol, unsigned long flags)
	:pqipersongrp(pol, flags) { return; }

	protected:

	/********* FUNCTIONS to OVERLOAD for specialisation ********/
virtual pqilistener *locked_createListener(struct sockaddr_in laddr);
virtual pqiperson   *locked_createPerson(std::string id, pqilistener *listener);
	/********* FUNCTIONS to OVERLOAD for specialisation ********/
};



#endif // MRK_PQI_PERSON_HANDLER_HEADER
