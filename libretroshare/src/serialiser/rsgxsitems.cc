/*
 * rsgxsitems.cc
 *
 *  Created on: 26 Jul 2012
 *      Author: crispy
 */


#include "rsgxsitems.h"
#include "gxs/rsgxsdata.h"

	void RsMsgMetaData::operator =(const RsGxsMsgMetaData& rGxsMeta)
	{
		this->mAuthorId = rGxsMeta.mAuthorId;
		this->mChildTs = rGxsMeta.mChildTs;
		this->mGroupId = rGxsMeta.mGroupId;
		this->mMsgFlags = rGxsMeta.mMsgFlags;
		this->mMsgId = rGxsMeta.mMsgId;
		this->mMsgName = rGxsMeta.mMsgName;
		this->mMsgStatus = rGxsMeta.mMsgStatus;
		this->mOrigMsgId = rGxsMeta.mOrigMsgId;
		this->mParentId = rGxsMeta.mParentId;
		this->mPublishTs = rGxsMeta.mPublishTs;
		this->mThreadId = rGxsMeta.mThreadId;
	}


	void RsGroupMetaData::operator =(const RsGxsGrpMetaData& rGxsMeta)
	{
		this->mAuthorId = rGxsMeta.mAuthorId;
		this->mGroupFlags = rGxsMeta.mGroupFlags;
		this->mGroupId = rGxsMeta.mGroupId;
		this->mGroupStatus = rGxsMeta.mGroupStatus;
		this->mLastPost = rGxsMeta.mLastPost;
		this->mMsgCount = rGxsMeta.mMsgCount;
		this->mPop = rGxsMeta.mPop;
		this->mPublishTs = rGxsMeta.mPublishTs;
		this->mSubscribeFlags = rGxsMeta.mSubscribeFlags;
		this->mGroupName = rGxsMeta.mGroupName;
	}
