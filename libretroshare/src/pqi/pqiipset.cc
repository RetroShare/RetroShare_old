/*
 * libretroshare/src/pqi: pqiipset.cc
 *
 * 3P/PQI network interface for RetroShare.
 *
 * Copyright 2007-2008 by Robert Fernie.
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

#include <time.h>
#include "pqi/pqiipset.h"
#include "util/rsstring.h"

bool pqiIpAddress::sameAddress(const pqiIpAddress &a) const
{
    	return ((mAddr.sin_addr.s_addr == a.mAddr.sin_addr.s_addr) && 
		(mAddr.sin_port == a.mAddr.sin_port));
}


bool pqiIpAddress::validAddress() const
{
	/* filter for unlikely addresses */
	if(isLoopbackNet(&(mAddr.sin_addr)))
	{
#ifdef IPADDR_DEBUG
		std::cerr << "pqiIpAddress::validAddress() ip parameter is loopback: disgarding." << std::endl ;
#endif
		return false;
	}

	if(mAddr.sin_addr.s_addr == 0 || mAddr.sin_addr.s_addr == 1 || mAddr.sin_port == 0) 
	{
#ifdef IPADDR_DEBUG
		std::cerr << "pqiIpAddress::validAddress() ip parameter is 0.0.0.0/1, or port is 0, ignoring." << std::endl;
#endif
		return false;
	}

	return true;

}


bool 	pqiIpAddrList::updateIpAddressList(const pqiIpAddress &addr)  
{
	std::list<pqiIpAddress>::iterator it;
	bool add = false;
	bool newAddr = true;

#ifdef IPADDR_DEBUG
	std::cerr << "pqiIpAddrList::updateIpAddressList()";
	std::cerr << std::endl;
#endif

	if (mAddrs.size() < MAX_ADDRESS_LIST_SIZE)
	{
#ifdef IPADDR_DEBUG
		std::cerr << "pqiIpAddrList::updateIpAddressList() small list: Add";
		std::cerr << std::endl;
#endif
		add = true;
	}
	else if (mAddrs.back().mSeenTime < addr.mSeenTime)
	{
#ifdef IPADDR_DEBUG
		std::cerr << "pqiIpAddrList::updateIpAddressList() oldAddr: Add";
		std::cerr << std::endl;
#endif
		add = true;
	}

	if ((!add) || (!addr.validAddress()))
	{
#ifdef IPADDR_DEBUG
		std::cerr << "pqiIpAddrList::updateIpAddressList() not Add or !valid.. fail";
		std::cerr << std::endl;
#endif
		return false;
	}

	for(it = mAddrs.begin(); it != mAddrs.end(); it++)
	{
		if (it->sameAddress(addr))
		{
#ifdef IPADDR_DEBUG
			std::cerr << "pqiIpAddrList::updateIpAddressList() found duplicate";
			std::cerr << std::endl;
#endif
			if (it->mSeenTime > addr.mSeenTime)
			{
#ifdef IPADDR_DEBUG
				std::cerr << "pqiIpAddrList::updateIpAddressList() orig better, returning";
				std::cerr << std::endl;
#endif
				/* already better -> quit */
				return false;
			}

#ifdef IPADDR_DEBUG
			std::cerr << "pqiIpAddrList::updateIpAddressList() deleting orig";
			std::cerr << std::endl;
#endif
			it = mAddrs.erase(it);
			newAddr = false;
			break;
		}
	}

	// ordered by decreaseing time. (newest at front)
	bool added = false;
	for(it = mAddrs.begin(); it != mAddrs.end(); it++)
	{
		if (it->mSeenTime < addr.mSeenTime)
		{
#ifdef IPADDR_DEBUG
			std::cerr << "pqiIpAddrList::updateIpAddressList() added orig SeenTime: " << it->mSeenTime << " new SeenTime: " << addr.mSeenTime;
			std::cerr << std::endl;
#endif

			added = true;
			mAddrs.insert(it, addr);
			break;
		}
	}
	if (!added)
	{
#ifdef IPADDR_DEBUG
		std::cerr << "pqiIpAddrList::updateIpAddressList() pushing to back";
		std::cerr << std::endl;
#endif
		mAddrs.push_back(addr);
	}

	/* pop if necessary */
	while (mAddrs.size() > MAX_ADDRESS_LIST_SIZE)
	{
#ifdef IPADDR_DEBUG
		std::cerr << "pqiIpAddrList::updateIpAddressList() popping back";
		std::cerr << std::endl;
#endif
		mAddrs.pop_back();
	}

	return newAddr;
}

void    pqiIpAddrList::extractFromTlv(const RsTlvIpAddrSet &tlvAddrs)
{
	std::list<RsTlvIpAddressInfo>::const_iterator it;

	for(it = tlvAddrs.addrs.begin(); it != tlvAddrs.addrs.end() ; ++it)
	{
		pqiIpAddress addr;
		addr.mAddr = it->addr;
		addr.mSeenTime = it->seenTime;
		addr.mSrc = it->source; 

		mAddrs.push_back(addr);
	}
}

void    pqiIpAddrList::loadTlv(RsTlvIpAddrSet &tlvAddrs)
{
	std::list<pqiIpAddress>::iterator it;

	for(it = mAddrs.begin(); it != mAddrs.end() ; ++it)
	{
		RsTlvIpAddressInfo addr;
		addr.addr = it->mAddr;
		addr.seenTime = it->mSeenTime;
		addr.source = it->mSrc;
	
		tlvAddrs.addrs.push_back(addr);
	}
}



void 	pqiIpAddrList::printIpAddressList(std::string &out) const
{
	std::list<pqiIpAddress>::const_iterator it;
	time_t now = time(NULL);
	for(it = mAddrs.begin(); it != mAddrs.end(); it++)
	{
		rs_sprintf_append(out, "%s:%u ( %ld old)\n", rs_inet_ntoa(it->mAddr.sin_addr).c_str(), ntohs(it->mAddr.sin_port), now - it->mSeenTime);
	}
	return;
}


bool    pqiIpAddrSet::updateLocalAddrs(const pqiIpAddress &addr)
{
	return	mLocal.updateIpAddressList(addr);
}

bool    pqiIpAddrSet::updateExtAddrs(const pqiIpAddress &addr)
{
	return	mExt.updateIpAddressList(addr);
}

bool    pqiIpAddrSet::updateAddrs(const pqiIpAddrSet &addrs)
{
#ifdef IPADDR_DEBUG
	std::cerr << "pqiIpAddrSet::updateAddrs()";
	std::cerr << std::endl;
#endif

	bool newAddrs = false;
	std::list<pqiIpAddress>::const_iterator it;
	for(it = addrs.mLocal.mAddrs.begin(); it != addrs.mLocal.mAddrs.end(); it++)
	{
		if (mLocal.updateIpAddressList(*it))
		{
#ifdef IPADDR_DEBUG
			std::cerr << "pqiIpAddrSet::updateAddrs() Updated Local Addr";
			std::cerr << std::endl;
#endif
			newAddrs = true;
		}
	}

	for(it = addrs.mExt.mAddrs.begin(); it != addrs.mExt.mAddrs.end(); it++)
	{
		if (mExt.updateIpAddressList(*it))
		{
#ifdef IPADDR_DEBUG
			std::cerr << "pqiIpAddrSet::updateAddrs() Updated Ext Addr";
			std::cerr << std::endl;
#endif
			newAddrs = true;
		}
	}
	return newAddrs;
}



void    pqiIpAddrSet::printAddrs(std::string &out) const
{
	out += "Local Addresses: ";
	mLocal.printIpAddressList(out);
	out += "\nExt Addresses: ";
	mExt.printIpAddressList(out);
	out += "\n";
}




