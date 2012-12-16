
/*
 * libretroshare/src/serialiser: rsdiscitems.cc
 *
 * RetroShare Serialiser.
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

#include "serialiser/rsbaseserial.h"
#include "serialiser/rsbaseserial.h"

#include "serialiser/rsserviceids.h"
#include "serialiser/rsdiscitems.h"

#include "serialiser/rstlvbase.h"
#include "serialiser/rstlvtypes.h"

/***
 * #define RSSERIAL_DEBUG 		1
 * #define RSSERIAL_ERROR_DEBUG 	1
 ***/

#define RSSERIAL_ERROR_DEBUG 		1

#include <iostream>

/*************************************************************************/

uint32_t    RsDiscSerialiser::size(RsItem *i)
{
        RsDiscAskInfo  *inf;
        RsDiscReply *rdr;
	RsDiscVersion *rdv;
        RsDiscHeartbeat *rdt;

	/* do reply first - as it is derived from Item */
	if (NULL != (rdr = dynamic_cast<RsDiscReply *>(i)))
	{
		return sizeReply(rdr);
	}
        else if (NULL != (inf = dynamic_cast<RsDiscAskInfo *>(i)))
	{
                return sizeAskInfo(inf);
	}
	else if (NULL != (rdv = dynamic_cast<RsDiscVersion *>(i)))
	{
		return sizeVersion(rdv);
	}
        else if (NULL != (rdt = dynamic_cast<RsDiscHeartbeat *>(i)))
        {
                return sizeHeartbeat(rdt);
        }

	return 0;
}

/* serialise the data to the buffer */
bool    RsDiscSerialiser::serialise(RsItem *i, void *data, uint32_t *pktsize)
{
        RsDiscAskInfo  *inf;
        RsDiscReply *rdr;
	RsDiscVersion *rdv;
        RsDiscHeartbeat *rdt;

	/* do reply first - as it is derived from Item */
	if (NULL != (rdr = dynamic_cast<RsDiscReply *>(i)))
	{
		return serialiseReply(rdr, data, pktsize);
	}
        else if (NULL != (inf = dynamic_cast<RsDiscAskInfo *>(i)))
	{
                return serialiseAskInfo(inf, data, pktsize);
	}
	else if (NULL != (rdv = dynamic_cast<RsDiscVersion *>(i)))
	{
		return serialiseVersion(rdv, data, pktsize);
	}
        else if (NULL != (rdt = dynamic_cast<RsDiscHeartbeat *>(i)))
        {
                return serialiseHeartbeat(rdt, data, pktsize);
        }

	return false;
}

RsItem *RsDiscSerialiser::deserialise(void *data, uint32_t *pktsize)
{
	/* get the type and size */
	uint32_t rstype = getRsItemId(data);

	if ((RS_PKT_VERSION_SERVICE != getRsItemVersion(rstype)) ||
		(RS_SERVICE_TYPE_DISC != getRsItemService(rstype)))
	{
		std::cerr << "RsDiscSerialiser::deserialise() Wrong Type" << std::endl;
		return NULL; /* wrong type */
	}

	switch(getRsItemSubType(rstype))
	{
		case RS_PKT_SUBTYPE_DISC_REPLY:
			return deserialiseReply(data, pktsize);
			break;
                case RS_PKT_SUBTYPE_DISC_ASK_INFO:
                        return deserialiseAskInfo(data, pktsize);
			break;
		case RS_PKT_SUBTYPE_DISC_VERSION:
			return deserialiseVersion(data, pktsize);
			break;
                case RS_PKT_SUBTYPE_DISC_HEARTBEAT:
                        return deserialiseHeartbeat(data, pktsize);
                        break;
                default:
			return NULL;
			break;
	}
	return NULL;
}

/*************************************************************************/

RsDiscAskInfo::~RsDiscAskInfo()
{
	return;
}

void 	RsDiscAskInfo::clear()
{
        gpg_id.clear();
}

std::ostream &RsDiscAskInfo::print(std::ostream &out, uint16_t indent)
{
        printRsItemBase(out, "RsDiscAskInfo", indent);
	uint16_t int_Indent = indent + 2;

        printIndent(out, int_Indent);
        out << "gpg_id: " << gpg_id << std::endl;

        printRsItemEnd(out, "RsDiscAskInfo", indent);
        return out;
}


uint32_t    RsDiscSerialiser::sizeAskInfo(RsDiscAskInfo *item)
{
	uint32_t s = 8; /* header */
        s += GetTlvStringSize(item->gpg_id);
	return s;
}

/* serialise the data to the buffer */
bool     RsDiscSerialiser::serialiseAskInfo(RsDiscAskInfo *item, void *data, uint32_t *pktsize)
{
        uint32_t tlvsize = sizeAskInfo(item);
	uint32_t offset = 0;

	if (*pktsize < tlvsize)
		return false; /* not enough space */

	*pktsize = tlvsize;

	bool ok = true;

	ok &= setRsItemHeader(data, tlvsize, item->PacketId(), tlvsize);

#ifdef RSSERIAL_DEBUG
        std::cerr << "RsDiscSerialiser::serialiseAskInfo() Header: " << ok << std::endl;
        std::cerr << "RsDiscSerialiser::serialiseAskInfo() Size: " << tlvsize << std::endl;
#endif

        /* skip the header */
	offset += 8;

	/* add mandatory parts first */
        ok &= SetTlvString(data, tlvsize, &offset, TLV_TYPE_STR_PEERID, item->gpg_id);

        if (offset != tlvsize) {
		ok = false;
#ifdef RSSERIAL_ERROR_DEBUG
                std::cerr << "RsDiscSerialiser::serialiseAskInfo() Size Error! " << std::endl;
#endif
	}

	return ok;
}

RsDiscAskInfo *RsDiscSerialiser::deserialiseAskInfo(void *data, uint32_t *pktsize) {
	/* get the type and size */
	uint32_t rstype = getRsItemId(data);
	uint32_t rssize = getRsItemSize(data);

	uint32_t offset = 0;


	if ((RS_PKT_VERSION_SERVICE != getRsItemVersion(rstype)) ||
		(RS_SERVICE_TYPE_DISC != getRsItemService(rstype)) ||
                (RS_PKT_SUBTYPE_DISC_ASK_INFO != getRsItemSubType(rstype)))
	{
#ifdef RSSERIAL_ERROR_DEBUG
                std::cerr << "RsDiscSerialiser::deserialiseAskInfo() Wrong Type" << std::endl;
#endif
		return NULL; /* wrong type */
	}

	if (*pktsize < rssize)    /* check size */
	{
#ifdef RSSERIAL_ERROR_DEBUG
                std::cerr << "RsDiscSerialiser::deserialiseAskInfo() Not Enough Space" << std::endl;
#endif
		return NULL; /* not enough data */
	}

	/* set the packet length */
	*pktsize = rssize;

	bool ok = true;

	/* ready to load */
	RsDiscAskInfo *item = new RsDiscAskInfo();
	item->clear();

	/* skip the header */
	offset += 8;

	/* get mandatory parts first */
        ok &= GetTlvString(data, rssize, &offset, TLV_TYPE_STR_PEERID, item->gpg_id);

        if (offset != rssize) {
#ifdef RSSERIAL_ERROR_DEBUG
                std::cerr << "RsDiscSerialiser::deserialiseAskInfo() offset != rssize" << std::endl;
#endif
		/* error */
		delete item;
		return NULL;
	}

        if (!ok) {
#ifdef RSSERIAL_ERROR_DEBUG
                std::cerr << "RsDiscSerialiser::deserialiseAskInfo() ok = false" << std::endl;
#endif
		delete item;
		return NULL;
	}

	return item;
}


/*************************************************************************/


RsDiscReply::~RsDiscReply()
{
	return;
}

void 	RsDiscReply::clear()
{
	aboutId.clear();
        certGPG.clear();
        rsPeerList.clear();
}

std::ostream &RsDiscReply::print(std::ostream &out, uint16_t indent)
{
        printRsItemBase(out, "RsDiscReply", indent);
	uint16_t int_Indent = indent + 2;

        printIndent(out, int_Indent);
        out << "AboutId:  " << aboutId  << std::endl;

        printIndent(out, int_Indent);
        out << "certGPG:  " << certGPG  << std::endl;

        printIndent(out, int_Indent);
        out << "RsDiscReply::print() RsPeerNetItem list : " << std::endl;
        for (std::list<RsPeerNetItem>::iterator pitemIt = rsPeerList.begin(); pitemIt!=(rsPeerList.end()); pitemIt++) {
            printIndent(out, int_Indent);
            pitemIt->print(std::cerr, indent);
        }

        printRsItemEnd(out, "RsDiscReply", indent);
        return out;
}


uint32_t    RsDiscSerialiser::sizeReply(RsDiscReply *item)
{
	uint32_t s = 8; /* header */
#ifdef RSSERIAL_DEBUG
	std::cerr << "RsDiscSerialiser::sizeReply() Header Size: " << s << std::endl;
#endif
	s += GetTlvStringSize(item->aboutId);

#ifdef RSSERIAL_DEBUG
	std::cerr << "RsDiscSerialiser::sizeReply() +AboutId Size: " << s << std::endl;
#endif
	s += GetTlvStringSize(item->certGPG);

#ifdef RSSERIAL_DEBUG
	std::cerr << "RsDiscSerialiser::sizeReply() +certGPG Size: " << s << std::endl;
#endif

	RsPeerConfigSerialiser rss ;

	for (std::list<RsPeerNetItem>::iterator it = item->rsPeerList.begin(); it != item->rsPeerList.end(); it++)
	{
		s += rss.size(&(*it)) ;
#ifdef RSSERIAL_DEBUG
		std::cerr << "RsDiscSerialiser::sizeReply() +RsPeerNetItem Size: " << s << std::endl;
#endif
	}

#ifdef RSSERIAL_DEBUG
	std::cerr << "RsDiscSerialiser::sizeReply() Total Size: " << s << std::endl;
#endif

	return s;
}

/* serialise the data to the buffer */
bool     RsDiscSerialiser::serialiseReply(RsDiscReply *item, void *data, uint32_t *pktsize)
{
	uint32_t tlvsize = sizeReply(item);
	uint32_t offset = 0;

#ifdef RSSERIAL_DEBUG
	std::cerr << "RsDiscSerialiser::serialiseReply() tlvsize: " << tlvsize;
	std::cerr << std::endl;
#endif

	if (*pktsize < tlvsize)
	{
#ifdef RSSERIAL_ERROR_DEBUG
		std::cerr << "RsDiscSerialiser::serialiseReply() ERROR not enough space" << std::endl;
		std::cerr << "RsDiscSerialiser::serialiseReply() ERROR *pktsize: " << *pktsize << " tlvsize: " << tlvsize;
		std::cerr << std::endl;
#endif
		return false; /* not enough space */
	}

	*pktsize = tlvsize;

	bool ok = true;

	ok &= setRsItemHeader(data, tlvsize, item->PacketId(), tlvsize);

#ifdef RSSERIAL_DEBUG
	std::cerr << "RsDiscSerialiser::serialiseReply() Header: " << ok << std::endl;
        std::cerr << "RsDiscSerialiser::serialiseReply() Size: " << tlvsize << std::endl;
#endif

	/* skip the header */
	offset += 8;

	/* add mandatory parts first */
	ok &= SetTlvString(data, tlvsize, &offset, TLV_TYPE_STR_PEERID, item->aboutId);
	ok &= SetTlvString(data, tlvsize, &offset, TLV_TYPE_STR_CERT_GPG, item->certGPG);

#ifdef RSSERIAL_DEBUG
	std::cerr << "RsDiscSerialiser::serialiseReply() Offset After Strings: " << offset << std::endl;
#endif

	//store the ip list
	RsPeerConfigSerialiser rss ;
	std::list<RsPeerNetItem>::iterator pitemIt;

	for (pitemIt = item->rsPeerList.begin(); pitemIt!=(item->rsPeerList.end()) && ok; ++pitemIt) 
	{
		uint32_t size = *pktsize - offset ;//~(uint32_t)0;	// we must be conservative otherwise the serialiser returns false !!

		ok &= rss.serialise(&(*pitemIt), (void *) (((char *) data) + offset), &size);

		// The size has been updated to its exact value.
		offset += size;

#ifdef RSSERIAL_DEBUG
		std::cerr << "RsDiscSerialiser::serialiseReply() RsPeerNetItem ok?: " << ok << std::endl;
		std::cerr << "RsDiscSerialiser::serialiseReply() Offset After RsPeerNetItem: " << offset << std::endl;
#endif

	}

	if (offset != tlvsize) 
	{
		ok = false;
#ifdef RSSERIAL_ERROR_DEBUG
		std::cerr << "RsDiscSerialiser::serialiseReply() Size Error: " << tlvsize << " != " << offset << std::endl;
#endif
	}

	return ok;
}

RsDiscReply *RsDiscSerialiser::deserialiseReply(void *data, uint32_t *pktsize)
{
	/* get the type and size */
	uint32_t rstype = getRsItemId(data);
	uint32_t rssize = getRsItemSize(data);

	uint32_t offset = 0;

#ifdef RSSERIAL_DEBUG
	std::cerr << "RsDiscSerialiser::deserialiseReply() Pkt Type: " << std::hex << rstype << std::dec;
	std::cerr << "RsDiscSerialiser::deserialiseReply() Pkt Size: " << rssize << std::endl;
#endif

	if ((RS_PKT_VERSION_SERVICE != getRsItemVersion(rstype)) ||
		(RS_SERVICE_TYPE_DISC != getRsItemService(rstype)) ||
		(RS_PKT_SUBTYPE_DISC_REPLY != getRsItemSubType(rstype)))
	{
#ifdef RSSERIAL_ERROR_DEBUG
		std::cerr << "RsDiscSerialiser::deserialiseReply() Wrong Type" << std::endl;
#endif
		return NULL; /* wrong type */
	}

	if (*pktsize < rssize)    /* check size */
	{
#ifdef RSSERIAL_ERROR_DEBUG
		std::cerr << "RsDiscSerialiser::deserialiseReply() pktsize != rssize" << std::endl;
#endif
		return NULL; /* not enough data */
	}

	/* set the packet length */
	*pktsize = rssize;

	bool ok = true;

	/* ready to load */
	RsDiscReply *item = new RsDiscReply();
	item->clear();

	/* skip the header */
	offset += 8;

	/* get mandatory parts first */
	ok &= GetTlvString(data, rssize, &offset, TLV_TYPE_STR_PEERID, item->aboutId);
	ok &= GetTlvString(data, rssize, &offset, TLV_TYPE_STR_CERT_GPG, item->certGPG);

#ifdef RSSERIAL_DEBUG
	std::cerr << "RsDiscSerialiser::deserialiseReply() offset after Strings: " << offset << std::endl;
#endif

	//get the peernet address list
	RsPeerConfigSerialiser rss ;

	while (offset < rssize) 
	{
		uint32_t peerNetSize = rssize - offset ;

		RsPeerNetItem *rsPeerNetItem = (RsPeerNetItem*)rss.deserialise((void *) (((char *) data) + offset), &peerNetSize);

		offset += peerNetSize;

#ifdef RSSERIAL_DEBUG
		std::cerr << "RsDiscSerialiser::deserialiseReply() offset aft PeerNetItem: " << offset << std::endl;
#endif

		if(rsPeerNetItem == NULL)
		{
#ifdef RSSERIAL_ERROR_DEBUG
			std::cerr << "RsDiscSerialiser::deserialiseReply() ERROR deserialise PeerNetItem Failed" << std::endl;
#endif
			break ;
		}
		
		item->rsPeerList.push_back(*rsPeerNetItem);
		delete rsPeerNetItem ;
	}

	if (offset != rssize) 
	{
#ifdef RSSERIAL_ERROR_DEBUG
		std::cerr << "RsDiscSerialiser::deserialiseReply() offset != rssize" << std::endl;
#endif
		/* error */
		delete item;
		return NULL;
	}

	if (!ok) 
	{
#ifdef RSSERIAL_ERROR_DEBUG
		std::cerr << "RsDiscSerialiser::deserialiseReply() ok = false" << std::endl;
#endif
		delete item;
		return NULL;
	}

	return item;
}

/*************************************************************************/


RsDiscVersion::~RsDiscVersion()
{
    return;
}
void RsDiscVersion::clear()
{
	version = "";
}

std::ostream &RsDiscVersion::print(std::ostream &out, uint16_t indent)
{
    printRsItemBase(out, "RsDiscVersion", indent);
	uint16_t int_Indent = indent + 2;

    printIndent(out, int_Indent);
    out << "Version String:  " << version  << std::endl;

    printRsItemEnd(out, "RsDiscVersion", indent);
    return out;
}

uint32_t RsDiscSerialiser::sizeVersion(RsDiscVersion *item)
{
    uint32_t s = 8; /* header */
    s += GetTlvStringSize(item->version);

	return s;
}

/* serialise the data to the buffer */
bool RsDiscSerialiser::serialiseVersion(RsDiscVersion *item, void *data, uint32_t *pktsize)
{
    uint32_t tlvsize = sizeVersion(item);
    uint32_t offset = 0;

    if (*pktsize < tlvsize)
        return false;   /* not enough space */

    *pktsize = tlvsize;

    bool ok = true;

    ok &= setRsItemHeader(data, *pktsize, item->PacketId(), *pktsize);

#ifdef RSSERIAL_DEBUG
	std::cerr << "RsDiscSerialiser::serialiseVersion() Header: " << ok << std::endl;
	std::cerr << "RsDiscSerialiser::serialiseVersion() Size: " << tlvsize << std::endl;
#endif

    /* skip the header */
    offset += 8;

    ok &= SetTlvString(data, tlvsize, &offset, TLV_TYPE_STR_VALUE, item->version);

    if (offset != tlvsize)
    {
        ok = false;
#ifdef RSSERIAL_ERROR_DEBUG
        std::cerr << "RsDiscSerialiser::serialiseVersion() Size Error! " << std::endl;
		std::cerr << "Offset: " << offset << " tlvsize: " << tlvsize << std::endl;
#endif
    }

    return ok;
}

RsDiscVersion *RsDiscSerialiser::deserialiseVersion(void *data, uint32_t *pktsize)
{
    /* get the type and size */
	uint32_t rstype = getRsItemId(data);
	uint32_t rssize = getRsItemSize(data);

	uint32_t offset = 0;

    if ((RS_PKT_VERSION_SERVICE != getRsItemVersion(rstype)) ||
		(RS_SERVICE_TYPE_DISC != getRsItemService(rstype)) ||
		(RS_PKT_SUBTYPE_DISC_VERSION != getRsItemSubType(rstype)))
	{
#ifdef RSSERIAL_ERROR_DEBUG
		std::cerr << "RsDiscSerialiser::deserialiseVersion() Wrong Type" << std::endl;
#endif
		return NULL; /* wrong type */
	}

	if (*pktsize < rssize)    /* check size */
	{
#ifdef RSSERIAL_ERROR_DEBUG
		std::cerr << "RsDiscSerialiser::deserialiseVersion() pktsize != rssize" << std::endl;
		std::cerr << "Pktsize: " << *pktsize << " Rssize: " << rssize << std::endl;
#endif
		return NULL; /* not enough data */
	}

	/* set the packet length */
	*pktsize = rssize;

	bool ok = true;

	/* ready to load */
	RsDiscVersion *item = new RsDiscVersion();
	item->clear();

	/* skip the header */
	offset += 8;

	ok &= GetTlvString(data, rssize, &offset, TLV_TYPE_STR_VALUE, item->version);

	if (offset != rssize)
	{
#ifdef RSSERIAL_ERROR_DEBUG
		std::cerr << "RsDiscSerialiser::deserialiseVersion() offset != rssize" << std::endl;
		std::cerr << "Offset: " << offset << " Rssize: " << rssize << std::endl;
#endif
		/* error */
		delete item;
		return NULL;
	}

	if (!ok)
	{
#ifdef RSSERIAL_ERROR_DEBUG
		std::cerr << "RsDiscSerialiser::deserialiseVersion() ok = false" << std::endl;
#endif
		delete item;
		return NULL;
	}

	return item;
}


/*************************************************************************/


RsDiscHeartbeat::~RsDiscHeartbeat()
{
    return;
}
void RsDiscHeartbeat::clear()
{
}

std::ostream &RsDiscHeartbeat::print(std::ostream &out, uint16_t indent)
{
    printRsItemBase(out, "RsDiscHeartbeat", indent);
    printRsItemEnd(out, "RsDiscHeartbeat", indent);
    return out;
}

uint32_t RsDiscSerialiser::sizeHeartbeat(RsDiscHeartbeat */*item*/)
{
    uint32_t s = 8; /* header */

        return s;
}

/* serialise the data to the buffer */
bool RsDiscSerialiser::serialiseHeartbeat(RsDiscHeartbeat *item, void *data, uint32_t *pktsize)
{
    uint32_t tlvsize = sizeHeartbeat(item);
    uint32_t offset = 0;

    if (*pktsize < tlvsize)
        return false;   /* not enough space */

    *pktsize = tlvsize;

    bool ok = true;

    ok &= setRsItemHeader(data, *pktsize, item->PacketId(), *pktsize);

#ifdef RSSERIAL_DEBUG
        std::cerr << "RsDiscSerialiser::serialiseHeartbeat() Header: " << ok << std::endl;
        std::cerr << "RsDiscSerialiser::serialiseHeartbeat() Size: " << tlvsize << std::endl;
#endif

    /* skip the header */
    offset += 8;

    if (offset != tlvsize)
    {
        ok = false;
#ifdef RSSERIAL_ERROR_DEBUG
        std::cerr << "RsDiscSerialiser::serialiseHeartbeat() Size Error! " << std::endl;
                std::cerr << "Offset: " << offset << " tlvsize: " << tlvsize << std::endl;
#endif
    }

    return ok;
}

RsDiscHeartbeat *RsDiscSerialiser::deserialiseHeartbeat(void *data, uint32_t *pktsize)
{
    /* get the type and size */
        uint32_t rstype = getRsItemId(data);
        uint32_t rssize = getRsItemSize(data);

        uint32_t offset = 0;

    if ((RS_PKT_VERSION_SERVICE != getRsItemVersion(rstype)) ||
                (RS_SERVICE_TYPE_DISC != getRsItemService(rstype)) ||
                (RS_PKT_SUBTYPE_DISC_HEARTBEAT != getRsItemSubType(rstype)))
        {
#ifdef RSSERIAL_ERROR_DEBUG
                std::cerr << "RsDiscSerialiser::deserialiseHeartbeat() Wrong Type" << std::endl;
#endif
                return NULL; /* wrong type */
        }

        if (*pktsize < rssize)    /* check size */
        {
#ifdef RSSERIAL_ERROR_DEBUG
                std::cerr << "RsDiscSerialiser::deserialiseHeartbeat() pktsize != rssize" << std::endl;
                std::cerr << "Pktsize: " << *pktsize << " Rssize: " << rssize << std::endl;
#endif
                return NULL; /* not enough data */
        }

        /* set the packet length */
        *pktsize = rssize;

        bool ok = true;

        /* ready to load */
        RsDiscHeartbeat *item = new RsDiscHeartbeat();
        item->clear();

        /* skip the header */
        offset += 8;

        if (offset != rssize)
        {
#ifdef RSSERIAL_ERROR_DEBUG
                std::cerr << "RsDiscSerialiser::deserialiseHeartbeat() offset != rssize" << std::endl;
                std::cerr << "Offset: " << offset << " Rssize: " << rssize << std::endl;
#endif
                /* error */
                delete item;
                return NULL;
        }

        if (!ok)
        {
#ifdef RSSERIAL_ERROR_DEBUG
                std::cerr << "RsDiscSerialiser::deserialiseHeartbeat() ok = false" << std::endl;
#endif
                delete item;
                return NULL;
        }

        return item;
}


/*************************************************************************/
