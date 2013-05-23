#ifndef RS_CHANNEL_GUI_INTERFACE_H
#define RS_CHANNEL_GUI_INTERFACE_H

/*
 * libretroshare/src/rsiface: rschannels.h
 *
 * RetroShare C++ Interface.
 *
 * Copyright 2008 by Robert Fernie.
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


#include <list>
#include <iostream>
#include <string>

#include "rstypes.h"
#include "rsdistrib.h" /* For FLAGS */

#define CHANNEL_MSG_STATUS_MASK           0x000f
#define CHANNEL_MSG_STATUS_READ           0x0001
#define CHANNEL_MSG_STATUS_UNREAD_BY_USER 0x0002
#define CHANNEL_MSG_STATUS_DOWLOADED      0x0004


//! Stores information for a give channel id
/*!
 * Stores all information for a given channel id
 */
class ChannelInfo 
{
	public:
    ChannelInfo() : autoDownload(false), pngChanImage(NULL), pngImageLen(0)
     {}
	std::string  channelId;
	std::wstring channelName;
	std::wstring channelDesc;

	uint32_t channelFlags;
	uint32_t pop; /// popularity
	bool autoDownload;

	unsigned char* pngChanImage;
	uint32_t pngImageLen;

	time_t lastPost;
	std::string destination_directory ;
};

//! for storing a channel msgs thumbnail picture
class ChannelMsgThumbnail
{
public:
	ChannelMsgThumbnail() : image_thumbnail(NULL), im_thumbnail_size(0) {}

	unsigned char* image_thumbnail;
	int im_thumbnail_size;
};

//! Stores information on a message within a channel
class ChannelMsgInfo 
{
	public:
	ChannelMsgInfo () : count(0), size(0) {}
	std::string channelId;
	std::string msgId;

	unsigned int msgflags;

	std::wstring subject;
	std::wstring msg;
	time_t ts; /// time stamp

	std::list<FileInfo> files;
	uint32_t count; /// file count
	uint64_t size; /// size of all files

	ChannelMsgThumbnail thumbnail;

};


//! gives a more brief account of a channel message than channelMsgInfo
class ChannelMsgSummary 
{
	public:
	ChannelMsgSummary() : count(0) {}
	std::string channelId;
	std::string msgId;

	uint32_t msgflags;

	std::wstring subject;
	std::wstring msg;
	uint32_t count; /// file count
	time_t ts; /// time stamp

};

std::ostream &operator<<(std::ostream &out, const ChannelInfo &info);
std::ostream &operator<<(std::ostream &out, const ChannelMsgSummary &info);
std::ostream &operator<<(std::ostream &out, const ChannelMsgInfo &info);

class RsChannels;
extern RsChannels   *rsChannels;

/*!
 * retroshare interface to the channels distributed group service
 * Channels user to create feeds similar to RSS feed where you can share files
 * with other users, when you subscribe to a channel you immediately begin downloading
 * the file shared on that channel. Channel feeds are shared anonymously
 */
class RsChannels 
{
	public:

	RsChannels() { return; }
virtual ~RsChannels() { return; }

/****************************************/

/*!
 *  returns a list of channel id that have changed (i.e. received new message, chan descr update)
 *  @param chanIds this is populated with channel ids that have changed
 */
virtual bool channelsChanged(std::list<std::string> &chanIds) = 0;

/*!
 * @param chanName name of the channel
 * @param chanDesc a short description for the created channel
 * @param chanFlags admin details on created channel group see rsdistrib.h for flags types
 * @param pngImageData point at image data in PNG format
 * @param imageSize size of the image data
 */
virtual std::string createChannel(std::wstring chanName, std::wstring chanDesc, uint32_t chanFlags,
		unsigned char* pngImageData, uint32_t imageSize) = 0;

/*!
 * retrieve channel information
 * @param cId channel id
 * @param ci channel info is store here
 */
virtual bool getChannelInfo(const std::string &cId, ChannelInfo &ci) = 0;

/*!
 * @param chanList populated channelinfo for all channels
 */
virtual bool getChannelList(std::list<ChannelInfo> &chanList) = 0;

/*!
 * get a message summary list for a given channel id
 * @param cId channel id user wants messages for
 * @param msgs summary of messages for the given cId
 */
virtual bool getChannelMsgList(const std::string &cId, std::list<ChannelMsgSummary> &msgs) = 0;

/*!
 * retrieve more comprehensive message info given channel id and message id
 */
virtual bool getChannelMessage(const std::string &cId, const std::string &mId, ChannelMsgInfo &msg) = 0;

/*!
 * set message status
 * @param cId channel id
 * @param mId message id
 * @param status status to set
 * @param statusMask bitmask to modify
 */
virtual bool setMessageStatus(const std::string& cId,const std::string& mId, const uint32_t status, const uint32_t statusMask) = 0;

/*!
 * set message status
 * @param cId channel id
 * @param mId message id
 * @param status status
 */
virtual bool getMessageStatus(const std::string& cId, const std::string& mId, uint32_t& status) = 0;

/*!
 * count the new and unread messages
 * @param cId channel id
 * @param newCount count of new messages
 * @param unreadCount count of unread messages
 */
virtual	bool getMessageCount(const std::string &cId, unsigned int &newCount, unsigned int &unreadCount) = 0;

/*!
 * send message contain in message info to the id indicated within it (make sure you set the channel id of the message info)
 * @param info message to be sent
 */
virtual	bool ChannelMessageSend(ChannelMsgInfo &info)                 = 0;

/*!
 * @param cId the channel id
 * @param subscribe set to true if you want to subscribe and to false to unsubscribe
 * @param set true to allow autodownload of new content and false otherwise, ignored when second param is false
 */
virtual bool channelSubscribe(const std::string &cId, bool subscribe, bool autoDl)	= 0;

/*!
 * This hashes a file which is not already shared by client or his peers,
 * The file is copied into the channels directory if its not too large (> 100mb)
 * @param path This is full path to file
 * @param channel Id
 */
virtual bool channelExtraFileHash(const std::string &path, const std::string &chId, FileInfo& fInfo) = 0;

/*!
 * This removes hashed extra files, and also removes channels directory copy if it exists
 * @param chId channel id
 */
virtual bool channelExtraFileRemove(const std::string &hash, const std::string &chId) = 0;

/*!
 * Restores channel private keys for channel in the event keys stored in configuration files are lost
 * @param chId channel id to restore keys for
 */
virtual bool channelRestoreKeys(const std::string &chId) = 0;

/*!
 * shares keys with peers
 *@param chId the channel for which private publish keys will be shared
 *@param peers  peers in this list will be sent keys
 *
 */
virtual bool channelShareKeys(const std::string &chId, std::list<std::string>& peers) = 0;
/****************************************/

/*!
 * allows peers to change information for the channel:
 * can only change channel image, descriptions and name
 *
 */
virtual bool channelEditInfo(const std::string &chId, ChannelInfo &ci) = 0;


/*!
 * get list of channels for which private publish key is available
 * @param grpIds list of channels for which private publish key is available
 */
virtual void getPubKeysAvailableGrpIds(std::list<std::string>& chanIds) = 0;

/*!
 * set the channel so that it does not auto download any more
 * @param chId the channel id to set for
 * @param set to true to enable auto dl and false to disable
 */
virtual bool channelSetAutoDl(const std::string& chId, bool  autoDl) = 0;

// sets the defautl destination directory for files downloaded in this channel.
// Default is "" which means Downloads/

virtual bool channelSetDestinationDirectory(const std::string& cid,const std::string& dir) = 0 ;

/*!
 * get what autoDl is set to for the given channel id
 * @param chId id of channel to get autoDl status for
 * @param autoDl
 * @return false if channel cannot be found
 */
virtual bool channelGetAutoDl(const std::string& chId, bool& autoDl) = 0;

};


#endif
