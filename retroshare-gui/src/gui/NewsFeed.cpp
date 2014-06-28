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

#include <QTimer>

#include "NewsFeed.h"

#include <retroshare/rsnotify.h>
#include <retroshare/rspeers.h>
//#include <retroshare/rschannels.h>
//#include <retroshare/rsforums.h>
#include <retroshare/rsmsgs.h>
#include <retroshare/rsplugin.h>

#if 0
#include "feeds/ChanNewItem.h"
#include "feeds/ChanMsgItem.h"
#include "feeds/ForumNewItem.h"
#include "feeds/ForumMsgItem.h"
#endif
#include "settings/rsettingswin.h"

#ifdef BLOGS
#include "feeds/BlogNewItem.h"
#include "feeds/BlogMsgItem.h"
#endif

#include "feeds/MsgItem.h"
#include "feeds/PeerItem.h"
#include "feeds/ChatMsgItem.h"
#include "feeds/SecurityItem.h"
#include "feeds/NewsFeedUserNotify.h"

#include "settings/rsharesettings.h"
#include "chat/ChatDialog.h"
#include "msgs/MessageComposer.h"
#include "common/FeedNotify.h"

const uint32_t NEWSFEED_PEERLIST = 	0x0001;

#if 0
const uint32_t NEWSFEED_FORUMNEWLIST = 	0x0002;
const uint32_t NEWSFEED_FORUMMSGLIST = 	0x0003;
const uint32_t NEWSFEED_CHANNEWLIST = 	0x0004;
const uint32_t NEWSFEED_CHANMSGLIST = 	0x0005;
const uint32_t NEWSFEED_BLOGNEWLIST = 	0x0006;
const uint32_t NEWSFEED_BLOGMSGLIST = 	0x0007;
#endif

const uint32_t NEWSFEED_MESSAGELIST = 	0x0008;
const uint32_t NEWSFEED_CHATMSGLIST = 	0x0009;
const uint32_t NEWSFEED_SECLIST = 	0x000a;

/*****
 * #define NEWS_DEBUG  1
 ****/

static NewsFeed *instance = NULL;

/** Constructor */
NewsFeed::NewsFeed(QWidget *parent)
: RsAutoUpdatePage(1000,parent)
{
	/* Invoke the Qt Designer generated object setup routine */
	setupUi(this);

	setUpdateWhenInvisible(true);

	if (!instance) {
		instance = this;
	}

	connect(removeAllButton, SIGNAL(clicked()), this, SLOT(removeAll()));
	connect(feedOptionsButton, SIGNAL(clicked()), this, SLOT(feedoptions()));

QString hlp_str = tr(
 " <h1><img width=\"32\" src=\":/images/64px_help.png\">&nbsp;&nbsp;News Feed</h1>                                                          \
   <p>The News Feed displays the last events on your network, sorted by the time you received them.                \
   This gives you a summary of the activity of your friends.                                                       \
   You can configure which events to show by pressing on <b>Options</b>. </p>                                      \
   <p>The various events shown are:                                                                                \
   <ul>                                                                                                         \
   <li>Connection attempts (useful to make friends with new people and control who's trying to reach you)</li> \
   <li>Channel and Forum posts</li>                                                                            \
   <li>New Channels and Forums you can subscribe to</li>                                                       \
   <li>Private messages from your friends</li>                                                                 \
   </ul> </p>                                                                                                      \
 ") ;

	registerHelpButton(helpButton,hlp_str) ;
}

NewsFeed::~NewsFeed()
{
	if (instance == this) {
		instance = NULL;
	}
}

UserNotify *NewsFeed::getUserNotify(QObject *parent)
{
	return new NewsFeedUserNotify(this, parent);
}

void NewsFeed::updateDisplay()
{
	if (!rsNotify)
		return;

	uint flags = Settings->getNewsFeedFlags();

	/* check for new messages */
	RsFeedItem fi;
	if (rsNotify->GetFeedItem(fi))
	{
		switch(fi.mType)
		{
			case RS_FEED_ITEM_PEER_CONNECT:
				if (flags & RS_FEED_TYPE_PEER)
					addFeedItemPeerConnect(fi);
				break;
			case RS_FEED_ITEM_PEER_DISCONNECT:
				if (flags & RS_FEED_TYPE_PEER)
					addFeedItemPeerDisconnect(fi);
				break;
			case RS_FEED_ITEM_PEER_NEW:
				if (flags & RS_FEED_TYPE_PEER)
					addFeedItemPeerNew(fi);
				break;
			case RS_FEED_ITEM_PEER_HELLO:
				if (flags & RS_FEED_TYPE_PEER)
					addFeedItemPeerHello(fi);
				break;

			case RS_FEED_ITEM_SEC_CONNECT_ATTEMPT:
			case RS_FEED_ITEM_SEC_WRONG_SIGNATURE:
			case RS_FEED_ITEM_SEC_BAD_CERTIFICATE:
			case RS_FEED_ITEM_SEC_MISSING_CERTIFICATE:
			case RS_FEED_ITEM_SEC_INTERNAL_ERROR:
				if (Settings->getMessageFlags() & RS_MESSAGE_CONNECT_ATTEMPT) {
					MessageComposer::sendConnectAttemptMsg(RsPgpId(fi.mId1), RsPeerId(fi.mId2), QString::fromUtf8(fi.mId3.c_str()));
				}
				if (flags & RS_FEED_TYPE_SECURITY)
					addFeedItemSecurityConnectAttempt(fi);
				break;
			case RS_FEED_ITEM_SEC_AUTH_DENIED:
				if (flags & RS_FEED_TYPE_SECURITY)
					addFeedItemSecurityAuthDenied(fi);
				break;
			case RS_FEED_ITEM_SEC_UNKNOWN_IN:
				if (flags & RS_FEED_TYPE_SECURITY)
					addFeedItemSecurityUnknownIn(fi);
				break;
			case RS_FEED_ITEM_SEC_UNKNOWN_OUT:
				if (flags & RS_FEED_TYPE_SECURITY)
					addFeedItemSecurityUnknownOut(fi);
				break;

#if 0
			case RS_FEED_ITEM_CHAN_NEW:
				if (flags & RS_FEED_TYPE_CHAN)
					addFeedItemChanNew(fi);
				break;
			case RS_FEED_ITEM_CHAN_UPDATE:
				if (flags & RS_FEED_TYPE_CHAN)
					addFeedItemChanUpdate(fi);
				break;
			case RS_FEED_ITEM_CHAN_MSG:
				if (flags & RS_FEED_TYPE_CHAN)
					addFeedItemChanMsg(fi);
				break;

			case RS_FEED_ITEM_FORUM_NEW:
				if (flags & RS_FEED_TYPE_FORUM)
					addFeedItemForumNew(fi);
				break;
			case RS_FEED_ITEM_FORUM_UPDATE:
				if (flags & RS_FEED_TYPE_FORUM)
					addFeedItemForumUpdate(fi);
				break;
			case RS_FEED_ITEM_FORUM_MSG:
				if (flags & RS_FEED_TYPE_FORUM)
					addFeedItemForumMsg(fi);
				break;

			case RS_FEED_ITEM_BLOG_NEW:
				if (flags & RS_FEED_TYPE_BLOG)
					addFeedItemBlogNew(fi);
				break;
			case RS_FEED_ITEM_BLOG_MSG:
				if (flags & RS_FEED_TYPE_BLOG)
					addFeedItemBlogMsg(fi);
				break;
#endif

			case RS_FEED_ITEM_CHAT_NEW:
				if (flags & RS_FEED_TYPE_CHAT)
					addFeedItemChatNew(fi, false);
				break;

			case RS_FEED_ITEM_MESSAGE:
				if (flags & RS_FEED_TYPE_MSG)
					addFeedItemMessage(fi);
				break;

			case RS_FEED_ITEM_FILES_NEW:
				if (flags & RS_FEED_TYPE_FILES)
					addFeedItemFilesNew(fi);
				break;
			default:
				std::cerr << "(EE) Unknown type " << std::hex << fi.mType << std::dec << " in news feed." << std::endl;
				break;
		}
	} else {
		/* process plugin feeds */
		int pluginCount = rsPlugins->nbPlugins();
		for (int i = 0; i < pluginCount; ++i) {
			RsPlugin *rsPlugin = rsPlugins->plugin(i);
			if (rsPlugin) {
				FeedNotify *feedNotify = rsPlugin->qt_feedNotify();
				if (feedNotify && feedNotify->notifyEnabled()) {
					QWidget *item = feedNotify->feedItem(this);
					if (item) {
						addFeedItem(item);
						break;
					}
				}
			}
		}
	}
}

void NewsFeed::testFeeds(uint notifyFlags)
{
	if (!instance) {
		return;
	}

	uint pos = 0;

	while (notifyFlags) {
		uint type = notifyFlags & (1 << pos);
		notifyFlags &= ~(1 << pos);
		++pos;

		RsFeedItem fi;

		switch(type) {
		case RS_FEED_TYPE_PEER:
			fi.mId1 = rsPeers->getOwnId().toStdString();

			instance->addFeedItemPeerConnect(fi);
			instance->addFeedItemPeerDisconnect(fi);
			instance->addFeedItemPeerNew(fi);
			instance->addFeedItemPeerHello(fi);
			break;

		case RS_FEED_TYPE_SECURITY:
			fi.mId1 = rsPeers->getGPGOwnId().toStdString();
			fi.mId2 = rsPeers->getOwnId().toStdString();

			instance->addFeedItemSecurityConnectAttempt(fi);
			instance->addFeedItemSecurityAuthDenied(fi);
			instance->addFeedItemSecurityUnknownIn(fi);
			instance->addFeedItemSecurityUnknownOut(fi);
			break;

#if 0
		case RS_FEED_TYPE_CHAN:
		{
			std::list<ChannelInfo> channelList;
			rsChannels->getChannelList(channelList);

			std::list<ChannelInfo>::iterator channelIt;
			for (channelIt = channelList.begin(); channelIt != channelList.end(); ++channelIt) {
				if (fi.mId1.empty()) {
					/* store first channel */
					fi.mId1 = channelIt->channelId;
				}

				if (!channelIt->channelDesc.empty()) {
					/* take channel with description */
					fi.mId1 = channelIt->channelId;
					break;
				}
			}

			instance->addFeedItemChanNew(fi);
			instance->addFeedItemChanUpdate(fi);

			RsFeedItem fiMsg;
			bool bFound = false;

			for (channelIt = channelList.begin(); channelIt != channelList.end(); ++channelIt) {
				std::list<ChannelMsgSummary> channelMsgs;
				rsChannels->getChannelMsgList(channelIt->channelId, channelMsgs);

				std::list<ChannelMsgSummary>::iterator msgIt;
				for (msgIt = channelMsgs.begin(); msgIt != channelMsgs.end(); ++msgIt) {
					if (fiMsg.mId2.empty()) {
						/* store first channel message */
						fiMsg.mId1 = msgIt->channelId;
						fiMsg.mId2 = msgIt->msgId;
					}

					if (!msgIt->msg.empty()) {
						/* take channel message with description */
						fiMsg.mId1 = msgIt->channelId;
						fiMsg.mId2 = msgIt->msgId;
						bFound = true;
						break;
					}
				}

				if (bFound) {
					break;
				}
			}

			instance->addFeedItemChanMsg(fiMsg);
			break;
		}

		case RS_FEED_TYPE_FORUM:
		{
			std::list<ForumInfo> forumList;
			rsForums->getForumList(forumList);

			std::list<ForumInfo>::iterator forumIt;
			for (forumIt = forumList.begin(); forumIt != forumList.end(); ++forumIt) {
				if (fi.mId1.empty()) {
					/* store first forum */
					fi.mId1 = forumIt->forumId;
				}

				if (!forumIt->forumDesc.empty()) {
					/* take forum with description */
					fi.mId1 = forumIt->forumId;
					break;
				}
			}

			instance->addFeedItemForumNew(fi);
			instance->addFeedItemForumUpdate(fi);

			RsFeedItem fiMsg;
			bool bFound = false;

			for (forumIt = forumList.begin(); forumIt != forumList.end(); ++forumIt) {
				std::list<ThreadInfoSummary> forumMsgs;
				rsForums->getForumThreadList(forumIt->forumId, forumMsgs);

				std::list<ThreadInfoSummary>::iterator msgIt;
				for (msgIt = forumMsgs.begin(); msgIt != forumMsgs.end(); ++msgIt) {
					if (fiMsg.mId2.empty()) {
						/* store first forum message */
						fiMsg.mId1 = msgIt->forumId;
						fiMsg.mId2 = msgIt->msgId;
					}

					if (!msgIt->msg.empty()) {
						/* take channel message with description */
						fiMsg.mId1 = msgIt->forumId;
						fiMsg.mId2 = msgIt->msgId;
						bFound = true;
						break;
					}
				}

				if (bFound) {
					break;
				}
			}

			instance->addFeedItemForumMsg(fiMsg);
			break;
		}

		case RS_FEED_TYPE_BLOG:
// not used
//			instance->addFeedItemBlogNew(fi);
//			instance->addFeedItemBlogMsg(fi);
			break;
#endif

		case RS_FEED_TYPE_CHAT:
			fi.mId1 = rsPeers->getOwnId().toStdString();
			fi.mId2 = tr("This is a test.").toUtf8().constData();

			instance->addFeedItemChatNew(fi, true);
			break;

		case RS_FEED_TYPE_MSG:
		{
			std::list<MsgInfoSummary> msgList;
			rsMsgs->getMessageSummaries(msgList);

			std::list<MsgInfoSummary>::const_iterator msgIt;
			for (msgIt = msgList.begin(); msgIt != msgList.end(); ++msgIt) {
				if (fi.mId1.empty()) {
					/* store first message */
                    fi.mId1 = msgIt->msgId.toStdString();
				}

				if (msgIt->msgflags & RS_MSG_TRASH) {
					continue;
				}

				if ((msgIt->msgflags & RS_MSG_BOXMASK) == RS_MSG_INBOX) {
					/* take message from inbox */
                    fi.mId1 = msgIt->msgId.toStdString();
					break;
				}
			}

			instance->addFeedItemMessage(fi);
			break;
		}

		case RS_FEED_TYPE_FILES:
// not used
//			instance->addFeedItemFilesNew(fi);
			break;
		}
	}
}

void NewsFeed::testFeed(FeedNotify *feedNotify)
{
	if (!instance) {
		return;
	}

	if (!feedNotify) {
		return;
	}

	QWidget *feedItem = feedNotify->testFeedItem(instance);
	if (!feedItem) {
		return;
	}

	instance->addFeedItem(feedItem);
}

void NewsFeed::addFeedItem(QWidget *item)
{
	static const unsigned int MAX_WIDGETS_SIZE = 500 ;

	item->setAttribute(Qt::WA_DeleteOnClose, true);

	connect(item, SIGNAL(destroyed(QObject*)), this, SLOT(itemDestroyed(QObject*)));
	widgets.push_back(item);

	// costly, but not really a problem here
	while(widgets.size() > MAX_WIDGETS_SIZE)
	{
		QWidget *item = dynamic_cast<QWidget*>(widgets.front()) ;
		
		if(item)
			item->close() ;
		widgets.pop_front() ;
	}

	sendNewsFeedChanged();

	lockLayout(NULL, true);

	if (Settings->getAddFeedsAtEnd()) {
		itemsLayout->addWidget(item);
	} else {
		itemsLayout->insertWidget(0, item);
	}
	item->show();

	lockLayout(item, false);
}

void NewsFeed::addFeedItemIfUnique(QWidget *item, int itemType, const RsPeerId &sslId, bool replace)
{
	foreach (QObject *itemObject, widgets) {
		SecurityItem *secitem = dynamic_cast<SecurityItem*>(itemObject);
		if ((secitem) && (secitem->isSame(sslId, itemType)))
		{
			if (!replace)
			{
				delete item;
				return;
			}
			else
			{
				secitem->close();
				break;
			}
		}
	}

	addFeedItem(item);
}

void	NewsFeed::addFeedItemPeerConnect(RsFeedItem &fi)
{
	/* make new widget */
    PeerItem *pi = new PeerItem(this, NEWSFEED_PEERLIST, RsPeerId(fi.mId1), PEER_TYPE_CONNECT, false);

	/* store */

	/* add to layout */
	addFeedItem(pi);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemPeerConnect()";
	std::cerr << std::endl;
#endif

}

void	NewsFeed::addFeedItemPeerDisconnect(RsFeedItem &fi)
{
	/* make new widget */
    PeerItem *pi = new PeerItem(this, NEWSFEED_PEERLIST, RsPeerId(fi.mId1), PEER_TYPE_STD, false);

	/* store */

	/* add to layout */
	addFeedItem(pi);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemPeerDisconnect()";
	std::cerr << std::endl;
#endif
}

void	NewsFeed::addFeedItemPeerHello(RsFeedItem &fi)
{
	/* make new widget */
    PeerItem *pi = new PeerItem(this, NEWSFEED_PEERLIST, RsPeerId(fi.mId1), PEER_TYPE_HELLO, false);

	/* store */

	/* add to layout */
	addFeedItem(pi);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemPeerHello()";
	std::cerr << std::endl;
#endif
}

void	NewsFeed::addFeedItemPeerNew(RsFeedItem &fi)
{
	/* make new widget */
    PeerItem *pi = new PeerItem(this, NEWSFEED_PEERLIST, RsPeerId(fi.mId1), PEER_TYPE_NEW_FOF, false);

	/* store */

	/* add to layout */
	addFeedItem(pi);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemPeerNew()";
	std::cerr << std::endl;
#endif
}

void	NewsFeed::addFeedItemSecurityConnectAttempt(RsFeedItem &fi)
{
	/* make new widget */
    SecurityItem *pi = new SecurityItem(this, NEWSFEED_SECLIST, RsPgpId(fi.mId1), RsPeerId(fi.mId2), fi.mId3, fi.mId4, fi.mType, false);

	/* store */

	/* add to layout */
    addFeedItemIfUnique(pi, fi.mType, RsPeerId(fi.mId2), false);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemSecurityConnectAttempt()";
	std::cerr << std::endl;
#endif
}

void	NewsFeed::addFeedItemSecurityAuthDenied(RsFeedItem &fi)
{
	/* make new widget */
    SecurityItem *pi = new SecurityItem(this, NEWSFEED_SECLIST, RsPgpId(fi.mId1), RsPeerId(fi.mId2), fi.mId3, fi.mId4, fi.mType, false);

	/* store */

	/* add to layout */
    addFeedItemIfUnique(pi, RS_FEED_ITEM_SEC_AUTH_DENIED, RsPeerId(fi.mId2), false);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemSecurityAuthDenied()";
	std::cerr << std::endl;
#endif
}

void	NewsFeed::addFeedItemSecurityUnknownIn(RsFeedItem &fi)
{
	/* make new widget */
    SecurityItem *pi = new SecurityItem(this, NEWSFEED_SECLIST, RsPgpId(fi.mId1), RsPeerId(fi.mId2), fi.mId3, fi.mId4, RS_FEED_ITEM_SEC_UNKNOWN_IN, false);

	/* store */

	/* add to layout */
    addFeedItemIfUnique(pi, RS_FEED_ITEM_SEC_UNKNOWN_IN, RsPeerId(fi.mId2), false);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemSecurityUnknownIn()";
	std::cerr << std::endl;
#endif
}

void	NewsFeed::addFeedItemSecurityUnknownOut(RsFeedItem &fi)
{
	/* make new widget */
    SecurityItem *pi = new SecurityItem(this, NEWSFEED_SECLIST, RsPgpId(fi.mId1), RsPeerId(fi.mId2), fi.mId3, fi.mId4, RS_FEED_ITEM_SEC_UNKNOWN_OUT, false);

	/* store */

	/* add to layout */
    addFeedItemIfUnique(pi, RS_FEED_ITEM_SEC_UNKNOWN_OUT, RsPeerId(fi.mId2), false);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemSecurityUnknownOut()";
	std::cerr << std::endl;
#endif
}

#if 0
void	NewsFeed::addFeedItemChanNew(RsFeedItem &fi)
{

	/* make new widget */
	ChanNewItem *cni = new ChanNewItem(this, NEWSFEED_CHANNEWLIST, fi.mId1, false, true);

	/* store in list */

	/* add to layout */
	addFeedItem(cni);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemChanNew()";
	std::cerr << std::endl;
#endif
}

void	NewsFeed::addFeedItemChanUpdate(RsFeedItem &fi)
{
	/* make new widget */
	ChanNewItem *cni = new ChanNewItem(this, NEWSFEED_CHANNEWLIST, fi.mId1, false, false);

	/* store in list */

	/* add to layout */
	addFeedItem(cni);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemChanUpdate()";
	std::cerr << std::endl;
#endif
}

void	NewsFeed::addFeedItemChanMsg(RsFeedItem &fi)
{
	/* make new widget */
	ChanMsgItem *cm = new ChanMsgItem(this, NEWSFEED_CHANMSGLIST, fi.mId1, fi.mId2, false);

	/* store in forum list */

	/* add to layout */
	addFeedItem(cm);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemChanMsg()";
	std::cerr << std::endl;
#endif
}

void	NewsFeed::addFeedItemForumNew(RsFeedItem &fi)
{
	/* make new widget */
	ForumNewItem *fni = new ForumNewItem(this, NEWSFEED_FORUMNEWLIST, fi.mId1, false, true);

	/* store in forum list */
	mForumNewItems.push_back(fni);

	/* add to layout */
	addFeedItem(fni);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemForumNew()";
	std::cerr << std::endl;
#endif
}

void	NewsFeed::addFeedItemForumUpdate(RsFeedItem &fi)
{
	/* make new widget */
	ForumNewItem *fni = new ForumNewItem(this, NEWSFEED_FORUMNEWLIST, fi.mId1, false, false);

	/* store in forum list */
	mForumNewItems.push_back(fni);

	/* add to layout */
	addFeedItem(fni);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemForumUpdate()";
	std::cerr << std::endl;
#endif
}

void	NewsFeed::addFeedItemForumMsg(RsFeedItem &fi)
{
	/* make new widget */
	ForumMsgItem *fm = new ForumMsgItem(this, NEWSFEED_FORUMMSGLIST, fi.mId1, fi.mId2, false);

	/* store in forum list */

	/* add to layout */
	addFeedItem(fm);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemForumMsg()";
	std::cerr << std::endl;
#endif
}

void	NewsFeed::addFeedItemBlogNew(RsFeedItem &fi)
{
#ifdef BLOGS
	/* make new widget */
	BlogNewItem *bni = new BlogNewItem(this, NEWSFEED_BLOGNEWLIST, fi.mId1, false, true);

	/* store in list */

	/* add to layout */
	addFeedItem(bni);
#else
	Q_UNUSED(fi);
#endif

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemBlogNew()";
	std::cerr << std::endl;
#endif
}

void	NewsFeed::addFeedItemBlogMsg(RsFeedItem &fi)
{
#ifdef BLOGS
	/* make new widget */
	BlogMsgItem *bm = new BlogMsgItem(this, NEWSFEED_BLOGMSGLIST, fi.mId1, fi.mId2, false);

	/* store in forum list */

	/* add to layout */
	addFeedItem(bm);
#else
	Q_UNUSED(fi);
#endif

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemBlogMsg()";
	std::cerr << std::endl;
#endif
}

#endif

void	NewsFeed::addFeedItemChatNew(RsFeedItem &fi, bool addWithoutCheck)
{
#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemChatNew()";
	std::cerr << std::endl;
#endif

	if (!addWithoutCheck && fi.mId1 == rsPeers->getOwnId().toStdString()) {
		/* chat message from myself */
		return;
	}

	/* make new widget */
    ChatMsgItem *cm = new ChatMsgItem(this, NEWSFEED_CHATMSGLIST, RsPeerId(fi.mId1), fi.mId2);

	/* store in forum list */

	/* add to layout */
	addFeedItem(cm);
}

void	NewsFeed::addFeedItemMessage(RsFeedItem &fi)
{
	/* make new widget */
	MsgItem *mi = new MsgItem(this, NEWSFEED_MESSAGELIST, fi.mId1, false);

	/* store in list */

	/* add to layout */
	addFeedItem(mi);

#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemMessage()";
	std::cerr << std::endl;
#endif
}

void	NewsFeed::addFeedItemFilesNew(RsFeedItem &/*fi*/)
{
#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::addFeedItemFilesNew()";
	std::cerr << std::endl;
#endif
}

/* FeedHolder Functions (for FeedItem functionality) */
QScrollArea *NewsFeed::getScrollArea()
{
	return scrollArea;
}

void NewsFeed::deleteFeedItem(QWidget *item, uint32_t /*type*/)
{
#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::deleteFeedItem()";
	std::cerr << std::endl;
#endif

	if (item) {
		item->close ();
	}
}

void NewsFeed::openChat(const RsPeerId &peerId)
{
#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::openChat()";
	std::cerr << std::endl;
#endif

    ChatDialog::chatFriend(peerId);
}

void NewsFeed::openComments(uint32_t /*type*/, const RsGxsGroupId &/*groupId*/, const RsGxsMessageId &/*msgId*/, const QString &/*title*/)
{
	std::cerr << "NewsFeed::openComments() Not Handled Yet";
	std::cerr << std::endl;
}

void NewsFeed::itemDestroyed(QObject *item)
{
	widgets.remove(item);
	sendNewsFeedChanged();
}

void NewsFeed::removeAll()
{
#ifdef NEWS_DEBUG
	std::cerr << "NewsFeed::removeAll()" << std::endl;
#endif

	foreach (QObject *item, widgets) {
		if (item) {
			item->deleteLater();
		}
	}
	widgets.clear();
}

void NewsFeed::sendNewsFeedChanged()
{
	int count = 0;

	foreach (QObject *item, widgets) {
		if (dynamic_cast<PeerItem*>(item) == NULL) {
			/* don't count PeerItem's */
			count++;
		}
	}

	emit newsFeedChanged(count);
}

void NewsFeed::feedoptions()
{
	RSettingsWin::showYourself(this, RSettingsWin::Notify);
}
