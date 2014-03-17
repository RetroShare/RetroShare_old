/*
 * libretroshare/src/gxs: rsgxsutil.cc
 *
 * RetroShare C++ Interface. Generic routines that are useful in GXS
 *
 * Copyright 2013-2013 by Christopher Evi-Parker
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

#include "rsgxsutil.h"
#include "retroshare/rsgxsflags.h"
#include "pqi/pqihash.h"


RsGxsMessageCleanUp::RsGxsMessageCleanUp(RsGeneralDataService* const dataService, uint32_t messageStorePeriod, uint32_t chunkSize)
: mDs(dataService), MESSAGE_STORE_PERIOD(messageStorePeriod), CHUNK_SIZE(chunkSize)
{

	std::map<RsGxsGroupId, RsGxsGrpMetaData*> grpMeta;
	mDs->retrieveGxsGrpMetaData(grpMeta);

	std::map<RsGxsGroupId, RsGxsGrpMetaData*>::iterator cit = grpMeta.begin();

	for(;cit != grpMeta.end(); cit++)
	{
		mGrpMeta.push_back(cit->second);
	}
}


bool RsGxsMessageCleanUp::clean()
{
	int i = 1;

	time_t now = time(NULL);

	while(!mGrpMeta.empty())
	{
		RsGxsGrpMetaData* grpMeta = mGrpMeta.back();
		const RsGxsGroupId& grpId = grpMeta->mGroupId;
		mGrpMeta.pop_back();
		GxsMsgReq req;
		GxsMsgMetaResult result;

		req[grpId] = std::vector<RsGxsMessageId>();
		mDs->retrieveGxsMsgMetaData(req, result);

		GxsMsgMetaResult::iterator mit = result.begin();

		req.clear();

		for(; mit != result.end(); mit++)
		{
			std::vector<RsGxsMsgMetaData*>& metaV = mit->second;
			std::vector<RsGxsMsgMetaData*>::iterator vit = metaV.begin();

			for(; vit != metaV.end(); )
			{
				RsGxsMsgMetaData* meta = *vit;

				// check if expired
				bool remove = (meta->mPublishTs + MESSAGE_STORE_PERIOD) < now;

				// check client does not want the message kept regardless of age
				remove &= !(meta->mMsgStatus & GXS_SERV::GXS_MSG_STATUS_KEEP);

				// if not subscribed remove messages (can optimise this really)
				remove = remove || (grpMeta->mSubscribeFlags & GXS_SERV::GROUP_SUBSCRIBE_NOT_SUBSCRIBED);

				if( remove )
				{
					req[grpId].push_back(meta->mMsgId);
				}

				delete meta;
				vit = metaV.erase(vit);
			}
		}

		mDs->removeMsgs(req);

		delete grpMeta;

		i++;
		if(i > CHUNK_SIZE) break;
	}

	return mGrpMeta.empty();
}

RsGxsIntegrityCheck::RsGxsIntegrityCheck(
		RsGeneralDataService* const dataService) :
		mDs(dataService), mDone(false), mIntegrityMutex("integrity")
{ }

void RsGxsIntegrityCheck::run()
{
	check();
}

bool RsGxsIntegrityCheck::check()
{

	// first take out all the groups
	std::map<RsGxsGroupId, RsNxsGrp*> grp;
	mDs->retrieveNxsGrps(grp, true, true);
	std::vector<RsGxsGroupId> grpsToDel;

	// compute hash and compare to stored value, if it fails then simply add it
	// to list
	std::map<RsGxsGroupId, RsNxsGrp*>::iterator git = grp.begin();
	for(; git != grp.end(); git++)
	{
		RsNxsGrp* grp = git->second;
        RsFileHash currHash;
		pqihash pHash;
		pHash.addData(grp->grp.bin_data, grp->grp.bin_len);
		pHash.Complete(currHash);

		if(currHash != grp->metaData->mHash) grpsToDel.push_back(grp->grpId);
		delete grp;
	}

	mDs->removeGroups(grpsToDel);

	// now messages
	GxsMsgReq msgsToDel;
	GxsMsgResult msgs;

	mDs->retrieveNxsMsgs(msgsToDel, msgs, false, true);

	GxsMsgResult::iterator mit = msgs.begin();

	for(; mit != msgs.begin(); mit++)
	{
		std::vector<RsNxsMsg*>& msgV = mit->second;
		std::vector<RsNxsMsg*>::iterator vit = msgV.begin();

		for(; vit != msgV.end(); vit++)
		{
			RsNxsMsg* msg = *vit;
            RsFileHash currHash;
			pqihash pHash;
			pHash.addData(msg->msg.bin_data, msg->msg.bin_len);
			pHash.Complete(currHash);

			if(currHash != msg->metaData->mHash) msgsToDel[msg->grpId].push_back(msg->msgId);
			delete msg;
		}
	}

	mDs->removeMsgs(msgsToDel);

	RsStackMutex stack(mIntegrityMutex);
	mDone = true;

	return true;
}

bool RsGxsIntegrityCheck::isDone()
{
	RsStackMutex stack(mIntegrityMutex);
	return mDone;
}

