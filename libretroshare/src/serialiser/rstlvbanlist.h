#ifndef RS_TLV_BANLIST_TYPES_H
#define RS_TLV_BANLIST_TYPES_H

/*
 * libretroshare/src/serialiser: rstlvbanlist.h
 *
 * RetroShare Serialiser.
 *
 * Copyright 2011 by Robert Fernie
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

/*******************************************************************
 * These are the Compound TLV structures that must be (un)packed.
 ******************************************************************/

#include <map>
#include "serialiser/rstlvtypes.h"
#include "util/rsnet.h"

#define RSDSDV_MAX_ROUTE_TABLE	1000

class RsTlvBanListEntry: public RsTlvItem
{
	public:
	 RsTlvBanListEntry();
virtual ~RsTlvBanListEntry() { return; }
virtual uint32_t TlvSize();
virtual void	 TlvClear();
virtual bool     SetTlv(void *data, uint32_t size, uint32_t *offset); /* serialise   */
virtual bool     GetTlv(void *data, uint32_t size, uint32_t *offset); /* deserialise */
virtual std::ostream &print(std::ostream &out, uint16_t indent);

	struct sockaddr_in addr;
	uint32_t level;
	uint32_t reason;
	uint32_t age;
};

class RsTlvBanList: public RsTlvItem
{
	public:
	 RsTlvBanList();
virtual ~RsTlvBanList() { return; }
virtual uint32_t TlvSize();
virtual void	 TlvClear();
virtual bool     SetTlv(void *data, uint32_t size, uint32_t *offset); /* serialise   */
virtual bool     GetTlv(void *data, uint32_t size, uint32_t *offset); /* deserialise */
virtual std::ostream &print(std::ostream &out, uint16_t indent);

	std::list<RsTlvBanListEntry> entries;
};


#endif

