
/*
 * libretroshare/src/serialiser: rstlvtypes.cc
 *
 * RetroShare Serialiser.
 *
 * Copyright 2007-2008 by Robert Fernie, Chris Parker
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

#include "rstlvaddrs.h"

#include "rstlvbase.h"
#include "rstlvtypes.h"
#include "rsbaseserial.h"
#include "util/rsprint.h"
#include <ostream>
#include <iomanip>
#include <iostream>


/************************************* RsTlvIpAddressInfo ************************************/

RsTlvIpAddressInfo::RsTlvIpAddressInfo()
	:RsTlvItem(), seenTime(0), source(0)
{
	sockaddr_clear(&addr);
	return;
}

void RsTlvIpAddressInfo::TlvClear()
{
	sockaddr_clear(&addr);
	seenTime = 0;
	source = 0;
}

uint32_t RsTlvIpAddressInfo::TlvSize()
{
	uint32_t s = TLV_HEADER_SIZE; /* header + IpAddr + 8 for time & 4 for size */

	s += GetTlvIpAddrPortV4Size(); 
	s += 8; // seenTime
	s += 4; // source

	return s;

}

bool  RsTlvIpAddressInfo::SetTlv(void *data, uint32_t size, uint32_t *offset) /* serialise   */
{
	/* must check sizes */
	uint32_t tlvsize = TlvSize();
	uint32_t tlvend  = *offset + tlvsize;

	if (size < tlvend)
		return false; /* not enough space */

	bool ok = true;

	/* start at data[offset] */
        /* add mandatory parts first */

	ok &= SetTlvBase(data, tlvend, offset, TLV_TYPE_ADDRESS_INFO, tlvsize);

	ok &= SetTlvIpAddrPortV4(data, tlvend, offset, TLV_TYPE_IPV4_LAST, &addr);
	ok &= setRawUInt64(data, tlvend, offset, seenTime);
	ok &= setRawUInt32(data, tlvend, offset, source);

	return ok;

}


bool  RsTlvIpAddressInfo::GetTlv(void *data, uint32_t size, uint32_t *offset) /* serialise   */
{
	if (size < *offset + TLV_HEADER_SIZE)
		return false;	
	
	uint16_t tlvtype = GetTlvType( &(((uint8_t *) data)[*offset])  );
	uint32_t tlvsize = GetTlvSize( &(((uint8_t *) data)[*offset])  );
	uint32_t tlvend = *offset + tlvsize;

	if (size < tlvend)    /* check size */
		return false; /* not enough space */

	if (tlvtype != TLV_TYPE_ADDRESS_INFO) /* check type */
		return false;

	bool ok = true;

	/* ready to load */
	TlvClear();

	/* skip the header */
	(*offset) += TLV_HEADER_SIZE;

	ok &= GetTlvIpAddrPortV4(data, tlvend, offset, TLV_TYPE_IPV4_LAST, &addr);
	ok &= getRawUInt64(data, tlvend, offset, &(seenTime));
	ok &= getRawUInt32(data, tlvend, offset, &(source));
   

	/***************************************************************************
	 * NB: extra components could be added (for future expansion of the type).
	 *            or be present (if this code is reading an extended version).
	 *
	 * We must chew up the extra characters to conform with TLV specifications
	 ***************************************************************************/
	if (*offset != tlvend)
	{
#ifdef TLV_DEBUG
		std::cerr << "RsTlvIpAddressInfo::GetTlv() Warning extra bytes at end of item";
		std::cerr << std::endl;
#endif
		*offset = tlvend;
	}

	return ok;
	
}


std::ostream &RsTlvIpAddressInfo::print(std::ostream &out, uint16_t indent)
{ 
	printBase(out, "RsTlvIpAddressInfo", indent);
	uint16_t int_Indent = indent + 2;

	printIndent(out, int_Indent);
	out << "Address:" << rs_inet_ntoa(addr.sin_addr);
        out << ":" << htons(addr.sin_port) << std::endl;

	printIndent(out, int_Indent);
	out << "SeenTime:" << seenTime;
	out << std::endl;

	printIndent(out, int_Indent);
	out << "Source:" << source;
	out << std::endl;
	
	printEnd(out, "RsTlvIpAddressInfo", indent);
	return out;
}




/************************************* RsTlvIpAddrSet ************************************/

void RsTlvIpAddrSet::TlvClear()
{
	addrs.clear();
}

uint32_t RsTlvIpAddrSet::TlvSize()
{

	uint32_t s = TLV_HEADER_SIZE; /* header */

	std::list<RsTlvIpAddressInfo>::iterator it;
	

	if(!addrs.empty())
	{

		for(it = addrs.begin(); it != addrs.end() ; ++it)
			s += it->TlvSize();

	}

	return s;
}

bool  RsTlvIpAddrSet::SetTlv(void *data, uint32_t size, uint32_t *offset) /* serialise   */
{
	/* must check sizes */
	uint32_t tlvsize = TlvSize();
	uint32_t tlvend  = *offset + tlvsize;

	if (size < tlvend)
		return false; /* not enough space */

	bool ok = true;

		/* start at data[offset] */
	ok &= SetTlvBase(data, tlvend, offset, TLV_TYPE_ADDRESS_SET , tlvsize);
	
	if(!addrs.empty())
	{
		std::list<RsTlvIpAddressInfo>::iterator it;

		for(it = addrs.begin(); it != addrs.end() ; ++it)
			ok &= it->SetTlv(data, size, offset);
	}
	

return ok;

}


bool  RsTlvIpAddrSet::GetTlv(void *data, uint32_t size, uint32_t *offset) /* serialise   */
{
	if (size < *offset + TLV_HEADER_SIZE)
		return false;	

	uint16_t tlvtype = GetTlvType( &(((uint8_t *) data)[*offset])  );
	uint32_t tlvsize = GetTlvSize( &(((uint8_t *) data)[*offset])  );
	uint32_t tlvend = *offset + tlvsize;

	if (size < tlvend)    /* check size */
		return false; /* not enough space */

	if (tlvtype != TLV_TYPE_ADDRESS_SET) /* check type */
		return false;

	bool ok = true;

	/* ready to load */
	TlvClear();

	/* skip the header */
	(*offset) += TLV_HEADER_SIZE;

        /* while there is TLV  */
        while((*offset) + 2 < tlvend)
        {
                /* get the next type */
                uint16_t tlvsubtype = GetTlvType( &(((uint8_t *) data)[*offset]) );

                switch(tlvsubtype)
                {
                        case TLV_TYPE_ADDRESS_INFO:
			{
				RsTlvIpAddressInfo addr;
				ok &= addr.GetTlv(data, size, offset);
				if (ok)
				{
					addrs.push_back(addr);
				}
			}
				break;
                        default:
                                ok &= SkipUnknownTlv(data, tlvend, offset);
                                break;

                }

                if (!ok)
			break;
	}
   

		
	/***************************************************************************
	 * NB: extra components could be added (for future expansion of the type).
	 *            or be present (if this code is reading an extended version).
	 *
	 * We must chew up the extra characters to conform with TLV specifications
	 ***************************************************************************/
	if (*offset != tlvend)
	{
#ifdef TLV_DEBUG
		std::cerr << "RsTlvIpAddrSet::GetTlv() Warning extra bytes at end of item";
		std::cerr << std::endl;
#endif
		*offset = tlvend;
	}

	return ok;
}

// prints out contents of RsTlvIpAddrSet
std::ostream &RsTlvIpAddrSet::print(std::ostream &out, uint16_t indent)
{
	printBase(out, "RsTlvIpAddrSet", indent);
	uint16_t int_Indent = indent + 2;

	std::list<RsTlvIpAddressInfo>::iterator it;
	for(it = addrs.begin(); it != addrs.end() ; ++it)
		it->print(out, int_Indent);

	printEnd(out, "RsTlvIpAddrSet", indent);
	return out;
}


/************************************* RsTlvIpAddressInfo ************************************/

