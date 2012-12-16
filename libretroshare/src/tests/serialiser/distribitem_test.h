#ifndef DISTRIBITEM_TEST_H_
#define DISTRIBITEM_TEST_H_

/*
 * libretroshare/src/serialiser: distribitem_test.h
 *
 * RetroShare Serialiser tests
 *
 * Copyright 2010 by Christopher Evi-Parker.
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

#include "serialiser/rsdistribitems.h"
#include "serialiser/rschannelitems.h"
#include "serialiser/rsforumitems.h"
#include "serialiser/rsblogitems.h"



RsSerialType* init_item(RsDistribGrp&);
RsSerialType* init_item(RsDistribGrpKey&);
RsSerialType* init_item(RsDistribSignedMsg&);
RsSerialType* init_item(RsChannelMsg&);
RsSerialType* init_item(RsForumMsg&);
RsSerialType* init_item(RsForumReadStatus&);
RsSerialType* init_item(RsBlogMsg&);

bool operator==(const RsDistribGrp& , const RsDistribGrp& );
bool operator==(const RsDistribGrpKey& , const RsDistribGrpKey& );
bool operator==(const RsDistribSignedMsg& , const RsDistribSignedMsg& );
bool operator==(const RsChannelMsg& , const RsChannelMsg& );
bool operator==(const RsForumMsg& , const RsForumMsg& );
bool operator==(const RsForumReadStatus&, const RsForumReadStatus& );
bool operator==(const RsBlogMsg& , const RsBlogMsg& );




#endif /* DISTRIBITEM_TEST_H_ */
