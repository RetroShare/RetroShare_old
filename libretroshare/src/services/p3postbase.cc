/*
 * libretroshare/src/services p3posted.cc
 *
 * Posted interface for RetroShare.
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

#include <retroshare/rsidentity.h>

#include "retroshare/rsgxsflags.h"
#include <stdio.h>
#include <math.h>

#include "services/p3postbase.h"
#include "serialiser/rsgxscommentitems.h"

// For Dummy Msgs.
#include "util/rsrandom.h"
#include "util/rsstring.h"

/****
 * #define POSTBASE_DEBUG 1
 ****/

#define POSTBASE_BACKGROUND_PROCESSING	0x0002
#define PROCESSING_START_PERIOD		30
#define PROCESSING_INC_PERIOD		15

#define POSTBASE_ALL_GROUPS 		0x0011
#define POSTBASE_UNPROCESSED_MSGS	0x0012
#define POSTBASE_ALL_MSGS 		0x0013
#define POSTBASE_BG_POST_META		0x0014
/********************************************************************************/
/******************* Startup / Tick    ******************************************/
/********************************************************************************/

p3PostBase::p3PostBase(RsGeneralDataService *gds, RsNetworkExchangeService *nes, RsGixs* gixs,
	RsSerialType* serviceSerialiser, uint16_t serviceType)
    : RsGenExchange(gds, nes, serviceSerialiser, serviceType, gixs, postBaseAuthenPolicy()), GxsTokenQueue(this), RsTickEvent(), mPostBaseMtx("PostBaseMtx")
{
	mBgProcessing = false;

	mCommentService = new p3GxsCommentService(this,  serviceType);
	RsTickEvent::schedule_in(POSTBASE_BACKGROUND_PROCESSING, PROCESSING_START_PERIOD);
}


uint32_t p3PostBase::postBaseAuthenPolicy()
{
	uint32_t policy = 0;
	uint32_t flag = 0;

	flag = GXS_SERV::MSG_AUTHEN_ROOT_AUTHOR_SIGN | GXS_SERV::MSG_AUTHEN_CHILD_AUTHOR_SIGN;
	RsGenExchange::setAuthenPolicyFlag(flag, policy, RsGenExchange::PUBLIC_GRP_BITS);

	flag |= GXS_SERV::MSG_AUTHEN_ROOT_PUBLISH_SIGN | GXS_SERV::MSG_AUTHEN_CHILD_PUBLISH_SIGN;
	RsGenExchange::setAuthenPolicyFlag(flag, policy, RsGenExchange::RESTRICTED_GRP_BITS);
	RsGenExchange::setAuthenPolicyFlag(flag, policy, RsGenExchange::PRIVATE_GRP_BITS);

	flag = 0;
	RsGenExchange::setAuthenPolicyFlag(flag, policy, RsGenExchange::GRP_OPTION_BITS);

	return policy;
}

void p3PostBase::notifyChanges(std::vector<RsGxsNotify *> &changes)
{
	std::cerr << "p3PostBase::notifyChanges()";
	std::cerr << std::endl;

	std::vector<RsGxsNotify *> changesForGUI;
	std::vector<RsGxsNotify *>::iterator it;

	for(it = changes.begin(); it != changes.end(); it++)
	{
	       RsGxsGroupChange *groupChange = dynamic_cast<RsGxsGroupChange *>(*it);
	       RsGxsMsgChange *msgChange = dynamic_cast<RsGxsMsgChange *>(*it);
	       if (msgChange)
	       {
			std::cerr << "p3PostBase::notifyChanges() Found Message Change Notification";
			std::cerr << std::endl;

			std::map<RsGxsGroupId, std::vector<RsGxsMessageId> > &msgChangeMap = msgChange->msgChangeMap;
			std::map<RsGxsGroupId, std::vector<RsGxsMessageId> >::iterator mit;
			for(mit = msgChangeMap.begin(); mit != msgChangeMap.end(); mit++)
			{
				std::cerr << "p3PostBase::notifyChanges() Msgs for Group: " << mit->first;
				std::cerr << std::endl;
				// To start with we are just going to trigger updates on these groups.
				// FUTURE OPTIMISATION.
				// It could be taken a step further and directly request these msgs for an update.
				addGroupForProcessing(mit->first);
			}
			delete msgChange;
	       }

	       /* pass on Group Changes to GUI */
		if (groupChange)
		{
			std::cerr << "p3PostBase::notifyChanges() Found Group Change Notification";
			std::cerr << std::endl;

			std::list<RsGxsGroupId> &groupList = groupChange->mGrpIdList;
			std::list<RsGxsGroupId>::iterator git;
			for(git = groupList.begin(); git != groupList.end(); git++)
			{
				std::cerr << "p3PostBase::notifyChanges() Incoming Group: " << *git;
				std::cerr << std::endl;
			}
			changesForGUI.push_back(groupChange);
		}
	}
	changes.clear();
	receiveHelperChanges(changesForGUI);

	std::cerr << "p3PostBase::notifyChanges() -> receiveChanges()";
	std::cerr << std::endl;
}

void	p3PostBase::service_tick()
{
	RsTickEvent::tick_events();
	GxsTokenQueue::checkRequests();

	mCommentService->comment_tick();

	return;
}

/********************************************************************************************/
/********************************************************************************************/

void p3PostBase::setMessageReadStatus(uint32_t& token, const RsGxsGrpMsgIdPair& msgId, bool read)
{
	uint32_t mask = GXS_SERV::GXS_MSG_STATUS_UNREAD | GXS_SERV::GXS_MSG_STATUS_UNPROCESSED;
	uint32_t status = GXS_SERV::GXS_MSG_STATUS_UNREAD;
	if (read)
	{
		status = 0;
	}

	setMsgStatusFlags(token, msgId, status, mask);

}


        // Overloaded from RsTickEvent for Event callbacks.
void p3PostBase::handle_event(uint32_t event_type, const std::string & /* elabel */)
{
	std::cerr << "p3PostBase::handle_event(" << event_type << ")";
	std::cerr << std::endl;

	// stuff.
	switch(event_type)
	{
		case POSTBASE_BACKGROUND_PROCESSING:
			background_tick();
			break;

		default:
			/* error */
			std::cerr << "p3PostBase::handle_event() Unknown Event Type: " << event_type;
			std::cerr << std::endl;
			break;
	}
}


/*********************************************************************************
 * Background Calculations.
 *
 * Get list of change groups from Notify....
 * this doesn't imclude your own submissions (at this point). 
 * So they will not be processed until someone else changes something.
 * TODO FIX: Must push for that change.
 *
 * Eventually, we should just be able to get the new messages from Notify, 
 * and only process them!
 */

void p3PostBase::background_tick()
{

#if 0
	{
		RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/
		if (mBgGroupList.empty())
		{
			background_requestAllGroups();
		}
	}
#endif

	background_requestUnprocessedGroup();

	RsTickEvent::schedule_in(POSTBASE_BACKGROUND_PROCESSING, PROCESSING_INC_PERIOD);

}

bool p3PostBase::background_requestAllGroups()
{
	std::cerr << "p3PostBase::background_requestAllGroups()";
	std::cerr << std::endl;

	uint32_t ansType = RS_TOKREQ_ANSTYPE_LIST;
	RsTokReqOptions opts;
	opts.mReqType = GXS_REQUEST_TYPE_GROUP_IDS;

	uint32_t token = 0;
	RsGenExchange::getTokenService()->requestGroupInfo(token, ansType, opts);
	GxsTokenQueue::queueRequest(token, POSTBASE_ALL_GROUPS);

	return true;
}


void p3PostBase::background_loadGroups(const uint32_t &token)
{
	/* get messages */
	std::cerr << "p3PostBase::background_loadGroups()";
	std::cerr << std::endl;

	std::list<RsGxsGroupId> groupList;
	bool ok = RsGenExchange::getGroupList(token, groupList);

	if (!ok)
	{
		return;
	}

	std::list<RsGxsGroupId>::iterator it;
	for(it = groupList.begin(); it != groupList.end(); it++)
	{
		addGroupForProcessing(*it);
	}
}


void p3PostBase::addGroupForProcessing(RsGxsGroupId grpId)
{
#ifdef POSTBASE_DEBUG
	std::cerr << "p3PostBase::addGroupForProcessing(" << grpId << ")";
	std::cerr << std::endl;
#endif // POSTBASE_DEBUG

	{
		RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/
		// no point having multiple lookups queued.
		if (mBgGroupList.end() == std::find(mBgGroupList.begin(), 
						mBgGroupList.end(), grpId))
		{
			mBgGroupList.push_back(grpId);
		}
	}
}


void p3PostBase::background_requestUnprocessedGroup()
{
#ifdef POSTBASE_DEBUG
	std::cerr << "p3PostBase::background_requestUnprocessedGroup()";
	std::cerr << std::endl;
#endif // POSTBASE_DEBUG


	RsGxsGroupId grpId;
	{
		RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/
		if (mBgProcessing)
		{
			std::cerr << "p3PostBase::background_requestUnprocessedGroup() Already Active";
			std::cerr << std::endl;
			return;
		}
		if (mBgGroupList.empty())
		{
			std::cerr << "p3PostBase::background_requestUnprocessedGroup() No Groups to Process";
			std::cerr << std::endl;
			return;
		}

		grpId = mBgGroupList.front();
		mBgGroupList.pop_front();
		mBgProcessing = true;
	}

	background_requestGroupMsgs(grpId, true);
}





void p3PostBase::background_requestGroupMsgs(const RsGxsGroupId &grpId, bool unprocessedOnly)
{
	std::cerr << "p3PostBase::background_requestGroupMsgs() id: " << grpId;
	std::cerr << std::endl;

	uint32_t ansType = RS_TOKREQ_ANSTYPE_DATA;
	RsTokReqOptions opts;
	opts.mReqType = GXS_REQUEST_TYPE_MSG_DATA;

	if (unprocessedOnly)
	{
		opts.mStatusFilter = GXS_SERV::GXS_MSG_STATUS_UNPROCESSED;
		opts.mStatusMask = GXS_SERV::GXS_MSG_STATUS_UNPROCESSED;
	}

	std::list<RsGxsGroupId> grouplist;
	grouplist.push_back(grpId);

	uint32_t token = 0;

	RsGenExchange::getTokenService()->requestMsgInfo(token, ansType, opts, grouplist);

	if (unprocessedOnly)
	{
		GxsTokenQueue::queueRequest(token, POSTBASE_UNPROCESSED_MSGS);
	}
	else
	{
		GxsTokenQueue::queueRequest(token, POSTBASE_ALL_MSGS);
	}
}




void p3PostBase::background_loadUnprocessedMsgs(const uint32_t &token)
{
	background_loadMsgs(token, true);
}


void p3PostBase::background_loadAllMsgs(const uint32_t &token)
{
	background_loadMsgs(token, false);
}


/* This function is generalised to support any collection of messages, across multiple groups */

void p3PostBase::background_loadMsgs(const uint32_t &token, bool unprocessed)
{
	/* get messages */
	std::cerr << "p3PostBase::background_loadMsgs()";
	std::cerr << std::endl;

	std::map<RsGxsGroupId, std::vector<RsGxsMsgItem*> > msgData;
	bool ok = RsGenExchange::getMsgData(token, msgData);

	if (!ok)
	{
		std::cerr << "p3PostBase::background_loadMsgs() Failed to getMsgData()";
		std::cerr << std::endl;

		/* cleanup */
		background_cleanup();
		return;

	}

	{
		RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/
		mBgStatsMap.clear();
		mBgIncremental = unprocessed;
	}

	std::map<RsGxsGroupId, std::vector<RsGxsMessageId> > postMap;

	// generate vector of changes to push to the GUI.
	std::vector<RsGxsNotify *> changes;
	RsGxsMsgChange *msgChanges = new RsGxsMsgChange(RsGxsNotify::TYPE_PROCESSED);


	RsGxsGroupId groupId;
	std::map<RsGxsGroupId, std::vector<RsGxsMsgItem*> >::iterator mit;
	std::vector<RsGxsMsgItem*>::iterator vit;
	for (mit = msgData.begin(); mit != msgData.end(); mit++)
	{
		  groupId = mit->first;
		  for (vit = mit->second.begin(); vit != mit->second.end(); vit++)
		  {
			RsGxsMessageId parentId = (*vit)->meta.mParentId;
			RsGxsMessageId threadId = (*vit)->meta.mThreadId;
				
	
			bool inc_counters = false;
			uint32_t vote_up_inc = 0;
			uint32_t vote_down_inc = 0;
			uint32_t comment_inc = 0;
	
			bool add_voter = false;
			RsGxsId voterId;
			RsGxsCommentItem *commentItem;
			RsGxsVoteItem    *voteItem;
	
			/* THIS Should be handled by UNPROCESSED Filter - but isn't */
			if (!IS_MSG_UNPROCESSED((*vit)->meta.mMsgStatus))
			{
				RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/
				if (mBgIncremental)
				{
					std::cerr << "p3PostBase::background_loadMsgs() Msg already Processed - Skipping";
					std::cerr << std::endl;
					std::cerr << "p3PostBase::background_loadMsgs() ERROR This should not happen";
					std::cerr << std::endl;
					delete(*vit);
					continue;
				}
			}
	
			/* 3 types expected: PostedPost, Comment and Vote */
			if (parentId.isNull())
			{
				/* we don't care about top-level (Posts) */
				std::cerr << "\tIgnoring TopLevel Item";
				std::cerr << std::endl;

				/* but we need to notify GUI about them */	
				msgChanges->msgChangeMap[mit->first].push_back((*vit)->meta.mMsgId);
			}
			else if (NULL != (commentItem = dynamic_cast<RsGxsCommentItem *>(*vit)))
			{
				/* comment - want all */
				/* Comments are counted by Thread Id */
				std::cerr << "\tProcessing Comment: " << commentItem;
				std::cerr << std::endl;
	
				inc_counters = true;
				comment_inc = 1;
			}
			else if (NULL != (voteItem = dynamic_cast<RsGxsVoteItem *>(*vit)))
			{
				/* vote - only care about direct children */
				if (parentId == threadId)
				{
					/* Votes are organised by Parent Id,
					 * ie. you can vote for both Posts and Comments
					 */
					std::cerr << "\tProcessing Vote: " << voteItem;
					std::cerr << std::endl;
	
					inc_counters = true;
					add_voter = true;
					voterId = voteItem->meta.mAuthorId;
	
					if (voteItem->mMsg.mVoteType == GXS_VOTE_UP)
					{
						vote_up_inc = 1;
					}
					else
					{
						vote_down_inc = 1;
					}
				}
			}
			else
			{
				/* unknown! */
				std::cerr << "p3PostBase::background_processNewMessages() ERROR Strange NEW Message:";
				std::cerr << std::endl;
				std::cerr << "\t" << (*vit)->meta;
				std::cerr << std::endl;
	
			}
	
			if (inc_counters)
			{
				RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/
	
				std::map<RsGxsMessageId, PostStats>::iterator sit = mBgStatsMap.find(threadId);
				if (sit == mBgStatsMap.end())
				{
					// add to map of ones to update.		
					postMap[groupId].push_back(threadId);	

					mBgStatsMap[threadId] = PostStats(0,0,0);
					sit = mBgStatsMap.find(threadId);
				}
		
				sit->second.comments += comment_inc;
				sit->second.up_votes += vote_up_inc;
				sit->second.down_votes += vote_down_inc;


				if (add_voter)
				{
					sit->second.voters.push_back(voterId);
				}
		
				std::cerr << "\tThreadId: " << threadId;
				std::cerr << " Comment Total: " << sit->second.comments;
				std::cerr << " UpVote Total: " << sit->second.up_votes;
				std::cerr << " DownVote Total: " << sit->second.down_votes;
				std::cerr << std::endl;
			}
		
			/* flag all messages as processed */
			if ((*vit)->meta.mMsgStatus & GXS_SERV::GXS_MSG_STATUS_UNPROCESSED)
			{
				uint32_t token_a;
				RsGxsGrpMsgIdPair msgId = std::make_pair(groupId, (*vit)->meta.mMsgId);
				RsGenExchange::setMsgStatusFlags(token_a, msgId, 0, GXS_SERV::GXS_MSG_STATUS_UNPROCESSED);
			}
			delete(*vit);
		}
	}

	/* push updates of new Posts */
	if (msgChanges->msgChangeMap.size() > 0)
	{
		std::cerr << "p3PostBase::background_processNewMessages() -> receiveChanges()";
		std::cerr << std::endl;

		changes.push_back(msgChanges);
	 	receiveHelperChanges(changes);
	}

	/* request the summary info from the parents */
	uint32_t token_b;
	uint32_t anstype = RS_TOKREQ_ANSTYPE_SUMMARY; 
	RsTokReqOptions opts;
	opts.mReqType = GXS_REQUEST_TYPE_MSG_META;
	RsGenExchange::getTokenService()->requestMsgInfo(token_b, anstype, opts, postMap);

	GxsTokenQueue::queueRequest(token_b, POSTBASE_BG_POST_META);
	return;
}


#define RSGXS_MAX_SERVICE_STRING	1024
bool encodePostCache(std::string &str, const PostStats &s)
{
	char line[RSGXS_MAX_SERVICE_STRING];

	snprintf(line, RSGXS_MAX_SERVICE_STRING, "%d %d %d", s.comments, s.up_votes, s.down_votes);

	str = line;
	return true;
}

bool extractPostCache(const std::string &str, PostStats &s)
{

	uint32_t iupvotes, idownvotes, icomments;
	if (3 == sscanf(str.c_str(), "%d %d %d", &icomments, &iupvotes, &idownvotes))
	{
		s.comments = icomments;
		s.up_votes = iupvotes;
		s.down_votes = idownvotes;
		return true;
	}
	return false;
}


void p3PostBase::background_updateVoteCounts(const uint32_t &token)
{
	std::cerr << "p3PostBase::background_updateVoteCounts()";
	std::cerr << std::endl;

	GxsMsgMetaMap parentMsgList;
	GxsMsgMetaMap::iterator mit;
	std::vector<RsMsgMetaData>::iterator vit;

	bool ok = RsGenExchange::getMsgMeta(token, parentMsgList);

	if (!ok)
	{
		std::cerr << "p3PostBase::background_updateVoteCounts() ERROR";
		std::cerr << std::endl;
		background_cleanup();
		return;
	}

	// generate vector of changes to push to the GUI.
	std::vector<RsGxsNotify *> changes;
	RsGxsMsgChange *msgChanges = new RsGxsMsgChange(RsGxsNotify::TYPE_PROCESSED);

	for(mit = parentMsgList.begin(); mit != parentMsgList.end(); mit++)
	{
		for(vit = mit->second.begin(); vit != mit->second.end(); vit++)
		{
			std::cerr << "p3PostBase::background_updateVoteCounts() Processing Msg(" << mit->first;
			std::cerr << ", " << vit->mMsgId << ")";
			std::cerr << std::endl;
	
			RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/
	
			/* extract current vote count */
			PostStats stats;
			if (mBgIncremental)
			{
				if (!extractPostCache(vit->mServiceString, stats))
				{
					if (!(vit->mServiceString.empty()))
					{
						std::cerr << "p3PostBase::background_updateVoteCounts() Failed to extract Votes";
						std::cerr << std::endl;
						std::cerr << "\tFrom String: " << vit->mServiceString;
						std::cerr << std::endl;
					}
				}
			}
	
			/* get increment */
			std::map<RsGxsMessageId, PostStats>::iterator it;
			it = mBgStatsMap.find(vit->mMsgId);
	
			if (it != mBgStatsMap.end())
			{
				std::cerr << "p3PostBase::background_updateVoteCounts() Adding to msgChangeMap: ";
				std::cerr << mit->first << " MsgId: " << vit->mMsgId;
				std::cerr << std::endl;

				stats.increment(it->second);
				msgChanges->msgChangeMap[mit->first].push_back(vit->mMsgId);
			}
			else
			{
				// warning.
				std::cerr << "p3PostBase::background_updateVoteCounts() Warning No New Votes found.";
				std::cerr << " For MsgId: " << vit->mMsgId;
				std::cerr << std::endl;
			}
	
			std::string str;
			if (!encodePostCache(str, stats))
			{
				std::cerr << "p3PostBase::background_updateVoteCounts() Failed to encode Votes";
				std::cerr << std::endl;
			}
			else
			{
				std::cerr << "p3PostBase::background_updateVoteCounts() Encoded String: " << str;
				std::cerr << std::endl;
				/* store new result */
				uint32_t token_c;
				RsGxsGrpMsgIdPair msgId = std::make_pair(vit->mGroupId, vit->mMsgId);
				RsGenExchange::setMsgServiceString(token_c, msgId, str);
			}
		}
	}

	if (msgChanges->msgChangeMap.size() > 0)
	{
		std::cerr << "p3PostBase::background_updateVoteCounts() -> receiveChanges()";
		std::cerr << std::endl;

		changes.push_back(msgChanges);
	 	receiveHelperChanges(changes);
	}

	// DONE!.
	background_cleanup();
	return;

}


bool p3PostBase::background_cleanup()
{
	std::cerr << "p3PostBase::background_cleanup()";
	std::cerr << std::endl;

	RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/

	// Cleanup.
	mBgStatsMap.clear();
	mBgProcessing = false;

	return true;
}


	// Overloaded from GxsTokenQueue for Request callbacks.
void p3PostBase::handleResponse(uint32_t token, uint32_t req_type)
{
	std::cerr << "p3PostBase::handleResponse(" << token << "," << req_type << ")";
	std::cerr << std::endl;

	// stuff.
	switch(req_type)
	{
		case POSTBASE_ALL_GROUPS:
			background_loadGroups(token);
			break;
		case POSTBASE_UNPROCESSED_MSGS:
			background_loadUnprocessedMsgs(token);
			break;
		case POSTBASE_ALL_MSGS:
			background_loadAllMsgs(token);
			break;
		case POSTBASE_BG_POST_META:
			background_updateVoteCounts(token);
			break;
		default:
			/* error */
			std::cerr << "p3PostBase::handleResponse() Unknown Request Type: " << req_type;
			std::cerr << std::endl;
			break;
	}
}

