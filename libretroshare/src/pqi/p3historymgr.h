#ifndef RS_P3_HISTORY_MGR_H
#define RS_P3_HISTORY_MGR_H

/*
 * libretroshare/src/services: p3historymgr.h
 *
 * RetroShare C++
 *
 * Copyright 2011 by Thunder.
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

#include <map>
#include <list>

#include "serialiser/rshistoryitems.h"
#include "retroshare/rshistory.h"
#include "pqi/p3cfgmgr.h"

class RsChatMsgItem;

//! handles history
/*!
 * The is a retroshare service which allows peers
 * to store the history of the chat messages
 */
class p3HistoryMgr: public p3Config
{
public:
	p3HistoryMgr();
	virtual ~p3HistoryMgr();

	/******** p3HistoryMgr *********/

	void addMessage(bool incoming, const std::string &chatPeerId, const std::string &peerId, const RsChatMsgItem *chatItem);

	/********* RsHistory ***********/

	bool getMessages(const std::string &chatPeerId, std::list<HistoryMsg> &msgs, uint32_t loadCount);
	bool getMessage(uint32_t msgId, HistoryMsg &msg);
	void clear(const std::string &chatPeerId);
	void removeMessages(const std::list<uint32_t> &msgIds);
	bool getEnable(bool ofPublic);
	void setEnable(bool forPublic, bool enable);
	uint32_t getSaveCount(bool ofPublic);
	void setSaveCount(bool forPublic, uint32_t count);

	/********* p3config ************/

	virtual RsSerialiser *setupSerialiser();
	virtual bool saveList(bool& cleanup, std::list<RsItem*>& saveData);
	virtual void saveDone();
	virtual bool loadList(std::list<RsItem*>& load);

private:
	uint32_t nextMsgId;
	std::map<std::string, std::map<uint32_t, RsHistoryMsgItem*> > mMessages;

	bool mPublicEnable;
	bool mPrivateEnable;

	uint32_t mPublicSaveCount;
	uint32_t mPrivateSaveCount;

	std::list<RsItem*> saveCleanupList; /* TEMPORARY LIST WHEN SAVING */

	RsMutex mHistoryMtx;
};

#endif
