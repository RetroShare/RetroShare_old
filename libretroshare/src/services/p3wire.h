/*
 * libretroshare/src/services: p3wire.h
 *
 * Wiki interface for RetroShare.
 *
 * Copyright 2012-2012 by Robert Fernie.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2.1 as published by the Free Software Foundation.
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

#ifndef P3_WIRE_SERVICE_HEADER
#define P3_WIRE_SERVICE_HEADER

#include "retroshare/rswire.h"
#include "gxs/rsgenexchange.h"

#include <map>
#include <string>

/* 
 * Wiki Service
 *
 *
 */

class p3Wire: public RsGenExchange, public RsWire
{
public:
    p3Wire(RsGeneralDataService* gds, RsNetworkExchangeService* nes);

protected:

virtual void notifyChanges(std::vector<RsGxsNotify*>& changes) ;

public:

virtual void service_tick();

        /* Specific Service Data */
virtual bool getGroupData(const uint32_t &token, std::vector<RsWireGroup> &groups);
virtual bool getPulseData(const uint32_t &token, std::vector<RsWirePulse> &pulses);

virtual bool createGroup(uint32_t &token, RsWireGroup &group);
virtual bool createPulse(uint32_t &token, RsWirePulse &pulse);

	private:

virtual void generateDummyData();
std::string genRandomId();

	RsMutex mWireMtx;


};

#endif 
