#ifndef RS_P3MSG_INTERFACE_H
#define RS_P3MSG_INTERFACE_H

/*
 * libretroshare/src/rsserver: p3msgs.h
 *
 * RetroShare C++ Interface.
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

#include "retroshare/rsmsgs.h"

class p3MsgService;
class p3ChatService;

class RsChatMsgItem;

//! provides retroshares chatservice and messaging service
/*!
 * Provides rs with the ability to send/receive messages, immediate status,
 * custom status, avatar and
 * chats (public(group) and private) to peers
 */
class p3Msgs: public RsMsgs 
{
   public:

          p3Msgs(p3MsgService *p3m, p3ChatService *p3c)
                 :mMsgSrv(p3m), mChatSrv(p3c) { return; }
	  virtual ~p3Msgs() { return; }

	  /****************************************/
	  /* Message Items */

	  /*!
	   * @param msgList ref to list summarising client's msgs
	   */
	  virtual bool getMessageSummaries(std::list<MsgInfoSummary> &msgList);
	  virtual bool getMessage(const std::string &mId, MessageInfo &msg);
	  virtual void getMessageCount(unsigned int *pnInbox, unsigned int *pnInboxNew, unsigned int *pnOutbox, unsigned int *pnDraftbox, unsigned int *pnSentbox, unsigned int *pnTrashbox);

	  virtual bool MessageSend(MessageInfo &info);
	  virtual bool SystemMessage(const std::wstring &title, const std::wstring &message, uint32_t systemFlag);
	  virtual bool MessageToDraft(MessageInfo &info, const std::string &msgParentId);
	  virtual bool MessageToTrash(const std::string &mid, bool bTrash);
	  virtual bool MessageDelete(const std::string &mid);
	  virtual bool MessageRead(const std::string &mid, bool unreadByUser);
	  virtual bool MessageReplied(const std::string &mid, bool replied);
	  virtual bool MessageForwarded(const std::string &mid, bool forwarded);
	  virtual bool MessageStar(const std::string &mid, bool star);
	  virtual bool getMsgParentId(const std::string &msgId, std::string &msgParentId);

	  virtual bool getMessageTagTypes(MsgTagType& tags);
	  virtual bool setMessageTagType(uint32_t tagId, std::string& text, uint32_t rgb_color);
	  virtual bool removeMessageTagType(uint32_t tagId);

	  virtual bool getMessageTag(const std::string &msgId, MsgTagInfo& info);
	  /* set == false && tagId == 0 --> remove all */
	  virtual bool setMessageTag(const std::string &msgId, uint32_t tagId, bool set);

	  virtual bool resetMessageStandardTagTypes(MsgTagType& tags);

	  /*!
	   * gets avatar from peer, image data in jpeg format
	   */
	  virtual void getAvatarData(const std::string& pid,unsigned char *& data,int& size);

	  /*!
	   * sets clients avatar, image data should be in jpeg format
	   */
	  virtual void setOwnAvatarData(const unsigned char *data,int size);

	  /*!
	   * retrieve clients avatar, image data in jpeg format
	   */
	  virtual void getOwnAvatarData(unsigned char *& data,int& size);

	  /*!
	   * sets clients custom status (e.g. "i'm tired")
	   */
	  virtual void setCustomStateString(const std::string&  status_string) ;

	  /*!
	   * retrieves client's custom status
	   */
	  virtual std::string getCustomStateString() ;

	  /*!
	   * retrieves peer's custom status
	   */
	  virtual std::string getCustomStateString(const std::string& peer_id) ;
	  

	  /*!
	   * public chat sent to all peers
	   */
	  virtual	bool	sendPublicChat(const std::wstring& msg);

	  /*!
	   * chat is sent to specifc peer
	   * @param id peer to send chat msg to
	   */
	  virtual	bool	sendPrivateChat(const std::string& id, const std::wstring& msg);

	  /*!
	   * returns the count of messages in public or private queue
	   * @param public or private queue
	   */
	  virtual	int    getPublicChatQueueCount();

	  /*!
	   * @param chats ref to list of received public chats is stored here
	   */
	  virtual	bool	getPublicChatQueue(std::list<ChatInfo> &chats);

	  /*!
	   * returns the count of messages in private queue
	   * @param public or private queue
	   */
	  virtual	int    getPrivateChatQueueCount(bool incoming);

	  /*!
	   * @param id's of available private chat messages
	   */
	  virtual	bool   getPrivateChatQueueIds(bool incoming, std::list<std::string> &ids);

	  /*!
	   * @param chats ref to list of received private chats is stored here
	   */
	  virtual	bool	getPrivateChatQueue(bool incoming, const std::string& id, std::list<ChatInfo> &chats);

	  /*!
	   * @param clear private chat queue
	   */
	  virtual	bool	clearPrivateChatQueue(bool incoming, const std::string& id);

	  /*!
	   * sends immediate status string to a specific peer, e.g. in a private chat
	   * @param peer_id peer to send status string to
	   * @param status_string immediate status to send
	   */
	  virtual void    sendStatusString(const std::string& peer_id, const std::string& status_string) ;

	  /*!
	   * sends immediate status to all peers
	   * @param status_string immediate status to send
	   */
	  virtual void    sendGroupChatStatusString(const std::string& status_string) ;

	  /****************************************/

	  virtual bool joinVisibleChatLobby(const ChatLobbyId& id) ;
	  virtual void getListOfNearbyChatLobbies(std::vector<VisibleChatLobbyRecord>& public_lobbies) ;
	  virtual bool getVirtualPeerId(const ChatLobbyId& id,std::string& vpid) ;
	  virtual bool isLobbyId(const std::string& virtual_peer_id,ChatLobbyId& lobby_id) ;
	  virtual void getChatLobbyList(std::list<ChatLobbyInfo, std::allocator<ChatLobbyInfo> >&) ;
	  virtual void invitePeerToLobby(const ChatLobbyId&, const std::string&) ;
	  virtual bool acceptLobbyInvite(const ChatLobbyId& id) ;
	  virtual void denyLobbyInvite(const ChatLobbyId& id) ;
	  virtual void getPendingChatLobbyInvites(std::list<ChatLobbyInvite>& invites) ;
	  virtual void unsubscribeChatLobby(const ChatLobbyId& lobby_id) ;
	  virtual bool setNickNameForChatLobby(const ChatLobbyId& lobby_id,const std::string&) ;
	  virtual bool getNickNameForChatLobby(const ChatLobbyId&,std::string& nick) ;
	  virtual bool setDefaultNickNameForChatLobby(const std::string&) ;
	  virtual bool getDefaultNickNameForChatLobby(std::string& nick) ;
	  virtual ChatLobbyId createChatLobby(const std::string& lobby_name,const std::string& lobby_topic,const std::list<std::string>& invited_friends,uint32_t privacy_type) ;

	  virtual bool createDistantChatInvite(const std::string& pgp_id,time_t time_of_validity,std::string& encrypted_string) ;
	  virtual bool getDistantChatInviteList(std::vector<DistantChatInviteInfo>& invites);
	  virtual bool initiateDistantChatConnexion(const std::string& encrypted_string,std::string& hash,uint32_t& error_code) ;
	  virtual uint32_t getDistantChatStatus(const std::string& hash) ;

   private:

	  p3MsgService  *mMsgSrv;
	  p3ChatService *mChatSrv;
};


#endif
