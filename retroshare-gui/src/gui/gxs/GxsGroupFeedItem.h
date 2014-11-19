/*
 * Retroshare Gxs Feed Item
 *
 * Copyright 2012-2013 by Robert Fernie.
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

#ifndef _GXS_GROUPFEEDITEM_H
#define _GXS_GROUPFEEDITEM_H

#include <QMetaType>

#include <retroshare/rsgxsifacehelper.h>
#include "gui/feeds/FeedItem.h"
#include "util/TokenQueue.h"
#include "gui/RetroShareLink.h"

#include <stdint.h>

class FeedHolder;
class RsGxsUpdateBroadcastBase;

class GxsGroupFeedItem : public FeedItem, public TokenResponse
{
	Q_OBJECT

public:
	/** Note parent can = NULL */
	GxsGroupFeedItem(FeedHolder *feedHolder, uint32_t feedId, const RsGxsGroupId &groupId, bool isHome, RsGxsIfaceHelper *iface, bool autoUpdate);
	virtual ~GxsGroupFeedItem();

	RsGxsGroupId groupId() { return mGroupId; }
	uint32_t feedId() { return mFeedId; }

protected:
	uint32_t nextTokenType() { return ++mNextTokenType; }
	bool initLoadQueue();

	/* load group data */
	void requestGroup();

	virtual bool isLoading();
	virtual void loadGroup(const uint32_t &token) = 0;
	virtual RetroShareLink::enumType getLinkType() = 0;
	virtual QString groupName() = 0;
	virtual void fillDisplay(RsGxsUpdateBroadcastBase *updateBroadcastBase, bool complete);

	/* TokenResponse */
	virtual void loadRequest(const TokenQueue *queue, const TokenRequest &req);

protected slots:
	void subscribe();
	void unsubscribe();
	void removeItem();
	void copyGroupLink();

protected:
	FeedHolder *mFeedHolder;
	uint32_t mFeedId;
	bool mIsHome;
	RsGxsIfaceHelper *mGxsIface;
	TokenQueue *mLoadQueue;

private slots:
	/* RsGxsUpdateBroadcastBase */
	void fillDisplaySlot(bool complete);

private:
	RsGxsGroupId mGroupId;
	RsGxsUpdateBroadcastBase *mUpdateBroadcastBase;
	uint32_t mNextTokenType;
	uint32_t mTokenTypeGroup;
};

Q_DECLARE_METATYPE(RsGxsGroupId)

#endif
