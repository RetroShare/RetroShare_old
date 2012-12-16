/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2008 Robert Fernie
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#ifndef _BLOGS_MSG_ITEM_DIALOG_H
#define _BLOGS_MSG_ITEM_DIALOG_H

#include "ui_BlogsMsgItem.h"
#include <stdint.h>


class FeedHolder;
class SubFileItem;

class BlogsMsgItem : public QWidget, private Ui::BlogsMsgItem
{
  Q_OBJECT

public:
  /** Default Constructor */
  BlogsMsgItem(FeedHolder *parent, uint32_t feedId, std::string peerId, std::string blogId, std::string msgId, bool isHome);

  /** Default Destructor */

	void updateItemStatic();
  	void small();

private slots:
	/* default stuff */
  	void gotoHome();
  	void removeItem();
	void toggle();

	void playMedia();
	void unsubscribeChannel();

	void updateItem();

private:
	FeedHolder *mParent;
	uint32_t mFeedId;

	std::string mBlogId;
	std::string mMsgId;
  std::string mPeerId;

	bool mIsHome;

};



#endif

