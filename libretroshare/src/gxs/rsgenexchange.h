#ifndef RSGENEXCHANGE_H
#define RSGENEXCHANGE_H

/*
 * libretroshare/src/gxs: rsgenexchange.h
 *
 * RetroShare C++ Interface.
 *
 * Copyright 2012-2012 by Christopher Evi-Parker, Robert Fernie
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

#include <queue>
#include <ctime>

#include "rsgxs.h"
#include "rsgds.h"
#include "rsnxs.h"
#include "retroshare/rsgxsiface.h"
#include "rsgxsdataaccess.h"
#include "rsnxsobserver.h"
#include "retroshare/rsgxsservice.h"
#include "serialiser/rsnxsitems.h"

template<class GxsItem, typename Identity = std::string>
class GxsPendingItem
{
public:
	GxsPendingItem(GxsItem item, Identity id) :
		mItem(item), mId(id), mAttempts(0)
	{}

	GxsPendingItem(const GxsPendingItem& gpsi)
	{
		this->mItem = gpsi.mItem;
		this->mId = gpsi.mId;
		this->mAttempts = gpsi.mAttempts;
	}

	bool operator==(const Identity& id)
	{
		return this->mId == id;
	}

	GxsItem mItem;
	Identity mId;
	uint8_t mAttempts;
};

class GxsGrpPendingSign
{
public:

	GxsGrpPendingSign(RsGxsGrpItem* item, uint32_t token): mLastAttemptTS(0), mStartTS(time(NULL)), mToken(token),
		mItem(item), mHaveKeys(false)
	{}

	time_t mLastAttemptTS, mStartTS;
	uint32_t mToken;
	RsGxsGrpItem* mItem;
	bool mHaveKeys; // mKeys->first == true if key present
	RsTlvSecurityKeySet mPrivateKeys;
	RsTlvSecurityKeySet mPublicKeys;
};

typedef std::map<RsGxsGroupId, std::vector<RsGxsMsgItem*> > GxsMsgDataMap;
typedef std::map<RsGxsGroupId, RsGxsGrpItem*> GxsGroupDataMap;
typedef std::map<RsGxsGrpMsgIdPair, std::vector<RsGxsMsgItem*> > GxsMsgRelatedDataMap;

/*!
 * This should form the parent class to \n
 * all gxs services. This provides access to service's msg/grp data \n
 * management/publishing/sync features
 *
 * Features: \n
 *         a. Data Access: \n
 *              Provided by handle to RsTokenService. This ensure consistency \n
 *              of requests and hiearchy of groups -> then messages which are \n
 *              sectioned by group ids. \n
 *              The one caveat is that redemption of tokens are done through \n
 *              the backend of this class \n
 *         b. Publishing: \n
 *              Methods are provided to publish msg and group items and also make \n
 *              changes to meta information of both item types \n
 *         c. Sync/Notification: \n
 *              Also notifications are made here on receipt of new data from \n
 *              connected peers
 */

class RsGixs;

class RsGenExchange : public RsNxsObserver, public RsThread, public RsGxsIface
{
public:

	/// used by class derived for RsGenExchange to indicate if service create passed or not
	enum ServiceCreate_Return { SERVICE_CREATE_SUCCESS, SERVICE_CREATE_FAIL, SERVICE_CREATE_FAIL_TRY_LATER } ;

    /*!
     * Constructs a RsGenExchange object, the owner ship of gds, ns, and serviceserialiser passes \n
     * onto the constructed object
     * @param gds Data service needed to act as store of message
     * @param ns Network service needed to synchronise data with rs peers
     * @param serviceSerialiser The users service needs this \n
     *        in order for gen exchange to deal with its data types
     * @param mServType This should be service type used by the serialiser
     * @param gixs This is used for verification of msgs and groups received by Gen Exchange using identities, set to NULL if \n
     *        identity verification is not wanted
     * @param authenPolicy This determines the authentication used for verfying authorship of msgs and groups
     */
    RsGenExchange(RsGeneralDataService* gds, RsNetworkExchangeService* ns,
                  RsSerialType* serviceSerialiser, uint16_t mServType, RsGixs* gixs = NULL, uint32_t authenPolicy = 0);

    virtual ~RsGenExchange();

    /** S: Observer implementation **/

    /*!
     * @param messages messages are deleted after function returns
     */
    void notifyNewMessages(std::vector<RsNxsMsg*>& messages);

    /*!
     * @param messages messages are deleted after function returns
     */
    void notifyNewGroups(std::vector<RsNxsGrp*>& groups);

    /** E: Observer implementation **/

    /*!
     * This is called by Gxs service runner
     * periodically, use to implement non
     * blocking calls
     */
    void tick();

    /*!
     * Any backgroup processing needed by
     */
    virtual void service_tick() = 0;

    /*!
     *
     * @return handle to token service handle for making
     * request to this gxs service
     */
    RsTokenService* getTokenService();

    void run();

    /*!
     * Policy bit pattern portion
     */
    enum PrivacyBitPos { PUBLIC_GRP_BITS, RESTRICTED_GRP_BITS, PRIVATE_GRP_BITS, GRP_OPTION_BITS } ;

    /*!
     * Convenience function for setting bit patterns of the individual privacy level authentication
     * policy and group options
     * @param flag the bit pattern (and policy) set for the privacy policy
     * @param authenFlag Only the policy portion chosen will be modified with 'flag',
     * the origianl flags in the indicated bit position (pos) are over-written
     * @param pos The policy bit portion to modify
     * @see PrivacyBitPos
     */
    static bool setAuthenPolicyFlag(const uint8_t& flag, uint32_t& authenFlag, const PrivacyBitPos& pos);

public:

    /** data access functions **/

    /*!
     * Retrieve group list for a given token
     * @param token
     * @param groupIds
     * @return false if token cannot be redeemed, if false you may have tried to redeem when not ready
     */
    bool getGroupList(const uint32_t &token, std::list<RsGxsGroupId> &groupIds);

    /*!
     * Retrieve msg list for a given token sectioned by group Ids
     * @param token token to be redeemed
     * @param msgIds a map of grpId -> msgList (vector)
     * @return false if could not redeem token
     */
    bool getMsgList(const uint32_t &token, GxsMsgIdResult &msgIds);

    /*!
     * Retrieve msg list for a given token for message related info
     * @param token token to be redeemed
     * @param msgIds a map of RsGxsGrpMsgIdPair -> msgList (vector)
     * @return false if could not redeem token
     */
    bool getMsgRelatedList(const uint32_t &token, MsgRelatedIdResult& msgIds);


    /*!
     * retrieve group meta data associated to a request token
     * @param token
     * @param groupInfo
     * @return false if could not redeem token
     */
    bool getGroupMeta(const uint32_t &token, std::list<RsGroupMetaData> &groupInfo);

    /*!
     * retrieves message meta data associated to a request token
     * @param token token to be redeemed
     * @param msgInfo the meta data to be retrieved for token store here
     */
    bool getMsgMeta(const uint32_t &token, GxsMsgMetaMap &msgInfo);

    /*!
     * Retrieve msg meta for a given token for message related info
     * @param token token to be redeemed
     * @param msgIds a map of RsGxsGrpMsgIdPair -> msgList (vector)
     * @return false if could not redeem token
     */
    bool getMsgRelatedMeta(const uint32_t &token, GxsMsgRelatedMetaMap& msgMeta);


    /*!
     * Gxs services should call this for automatic handling of
     * changes, send
     * @param changes
     */
    virtual void receiveChanges(std::vector<RsGxsNotify*>& changes);

    /*!
     * Checks to see if a change has been received for
     * for a message or group
     * @param willCallGrpChanged if this is set to true, group changed function will return list
     *        groups that have changed, if false, the group changed list is cleared
     * @param willCallMsgChanged if this is set to true, msgChanged function will return map
     *        messages that have changed, if false, the message changed map is cleared
     * @return true if a change has occured for msg or group
     * @see groupsChanged
     * @see msgsChanged
     */
    bool updated(bool willCallGrpChanged = false, bool willCallMsgChanged = false);

    /*!
     * The groups changed. \n
     * class can reimplement to use to tailor
     * the group actually set for ui notification.
     * If receivedChanges is not passed RsGxsNotify changes
     * this function does nothing
     * @param grpIds returns list of grpIds that have changed
     * @see updated
     */
    void groupsChanged(std::list<RsGxsGroupId>& grpIds);

    /*!
     * The msg changed. \n
     * class can reimplement to use to tailor
     * the msg actually set for ui notification.
     * If receivedChanges is not passed RsGxsNotify changes
     * this function does nothing
     * @param msgs returns map of message ids that have changed
     * @see updated
     */
    void msgsChanged(std::map<RsGxsGroupId,
                             std::vector<RsGxsMessageId> >& msgs);


    bool subscribeToGroup(uint32_t& token, const RsGxsGroupId& grpId, bool subscribe);

protected:

    /*!
     * @param grpItem
     * @deprecated only here temporarily for testing
     */
    void createDummyGroup(RsGxsGrpItem* grpItem);

    /*!
     * retrieves group data associated to a request token
     * @param token token to be redeemed for grpitem retrieval
     * @param grpItem the items to be retrieved for token are stored here
     */
    bool getGroupData(const uint32_t &token, std::vector<RsGxsGrpItem*>& grpItem);

public:
    /*!
     * retrieves message data associated to a request token
     * @param token token to be redeemed for message item retrieval
     * @param msgItems
     */
    bool getMsgData(const uint32_t &token, GxsMsgDataMap& msgItems);

    /*!
     * retrieves message related data associated to a request token
     * @param token token to be redeemed for message item retrieval
     * @param msgItems
     */
    bool getMsgRelatedData(const uint32_t &token, GxsMsgRelatedDataMap& msgItems);

protected:

    /*!
     * Convenience template function for retrieve
     * msg related data from
     * @param GxsMsgType This represent derived msg class type of the service (i.e. msg type that derives from RsGxsMsgItem
     * @param MsgType Represents the final type the core data is converted to
     * @param token token to be redeemed
     */
    template <class GxsMsgType, class MsgType>
    bool getMsgRelatedDataT(const uint32_t &token, std::map<RsGxsGrpMsgIdPair, std::vector<MsgType> > &msgItems)
    {

        RsStackMutex stack(mGenMtx);
        NxsMsgRelatedDataResult msgResult;
        bool ok = mDataAccess->getMsgRelatedData(token, msgResult);
        NxsMsgRelatedDataResult::iterator mit = msgResult.begin();

        if(ok)
        {
            for(; mit != msgResult.end(); mit++)
            {
                std::vector<MsgType> gxsMsgItems;
                const RsGxsGrpMsgIdPair& msgId = mit->first;
                std::vector<RsNxsMsg*>& nxsMsgsV = mit->second;
                std::vector<RsNxsMsg*>::iterator vit
                = nxsMsgsV.begin();
                for(; vit != nxsMsgsV.end(); vit++)
                {
                    RsNxsMsg*& msg = *vit;

                    RsItem* item = mSerialiser->deserialise(msg->msg.bin_data,
                                    &msg->msg.bin_len);
                    GxsMsgType* mItem = dynamic_cast<GxsMsgType*>(item);

                    if(mItem == NULL)
                    {
                        delete msg;
                        continue;
                    }

                    mItem->meta = *((*vit)->metaData); // get meta info from nxs msg
                  //  GxsMsgType m = (*mItem); // doesn't work! don't know why, even with overloading done.
                    MsgType theServMsg = (MsgType)*mItem;
                    gxsMsgItems.push_back(theServMsg);
                    delete msg;
                }
                msgItems[msgId] = gxsMsgItems;
            }
        }
        return ok;
    }

public:
    /*!
     * Assigns a token value to passed integer
     * The status of the token can still be queried from request status feature
     * @warning the token space is shared with RsGenExchange backend, so do not
     * modify tokens except does you have created by calling generatePublicToken()
     * @return  token
     */
    uint32_t generatePublicToken();

    /*!
     * Updates the status of associate token
     * @warning the token space is shared with RsGenExchange backend, so do not
     * modify tokens except does you have created by calling generatePublicToken()
     * @param token
     * @param status
     * @return false if token could not be found, true if token disposed of
     */
    bool updatePublicRequestStatus(const uint32_t &token, const uint32_t &status);

    /*!
     * This gets rid of a publicly issued token
     * @param token
     * @return false if token could not found, true if token is disposed of
     */
    bool disposeOfPublicToken(const uint32_t &token);

protected:
    /*!
     * This gives access to the data store which hold msgs and groups
     * for the service
     * @return Data store for retrieving msgs and groups
     */
    RsGeneralDataService* getDataStore();

    /*!
     * Retrieve keys for a given group, \n
     * call is blocking retrieval from underlying db
     * @warning under normal circumstance a service should not need this
     * @param grpId the id of the group to retrieve keys for
     * @param keys set to the retrieved keys
     * @return false if group does not exist or grpId is empty
     */
    bool getGroupKeys(const RsGxsGroupId& grpId, RsTlvSecurityKeySet& keySet);

public:

    /*!
     * This allows the client service to acknowledge that their msgs has \n
     * been created/modified and retrieve the create/modified msg ids
     * @param token the token related to modification/create request
     * @param msgIds map of grpid->msgIds of message created/modified
     * @return true if token exists false otherwise
     */
    bool acknowledgeTokenMsg(const uint32_t& token, RsGxsGrpMsgIdPair& msgId);

    /*!
	 * This allows the client service to acknowledge that their grps has \n
	 * been created/modified and retrieve the create/modified grp ids
	 * @param token the token related to modification/create request
	 * @param msgIds vector of ids of groups created/modified
	 * @return true if token exists false otherwise
	 */
    bool acknowledgeTokenGrp(const uint32_t& token, RsGxsGroupId& grpId);

protected:

    /** Modifications **/

    /*!
     * Enables publication of a group item \n
     * If the item exists already this is simply versioned \n
     * This will induce a related change message \n
     * Ownership of item passes to this rsgenexchange \n
     * @param token
     * @param grpItem
     */
    void publishGroup(uint32_t& token, RsGxsGrpItem* grpItem);

public:
    /*!
     * Enables publication of a message item \n
     * Setting mOrigMsgId meta member to blank \n
     * leads to this msg being an original msg \n
     * if mOrigMsgId is not blank the msgId then this msg is \n
     * considered a versioned msg \n
     * Ownership of item passes to this rsgenexchange
     * @param token
     * @param msgItem
     */
    void publishMsg(uint32_t& token, RsGxsMsgItem* msgItem);

protected:
    /*!
     * This represents the group before its signature is calculated
     * Reimplement this function if you need to access keys to further extend
     * security of your group items using keyset properties
     * Derived service should return one of three ServiceCreate_Return enum values below
     * @warning do not modify keySet!
     * @param grp The group which is stored by GXS prior
     *            service can make specific modifications need
     *            in particular access to its keys and meta
     * @param keySet this is the key set used to define the group
     *               contains private and public admin and publish keys
     *               (use key flags to distinguish)
     * @return SERVICE_CREATE_SUCCESS, SERVICE_CREATE_FAIL, SERVICE_FAIL_TRY_LATER
     */
    virtual ServiceCreate_Return service_CreateGroup(RsGxsGrpItem* grpItem, RsTlvSecurityKeySet& keySet);

public:

    /*!
     * sets the group subscribe flag
     * @param token this is set to token value associated to this request
     * @param grpId Id of group whose subscribe file will be changed
     * @param status
     * @param mask
     */
    void setGroupSubscribeFlags(uint32_t& token, const RsGxsGroupId& grpId, const uint32_t& status, const uint32_t& mask);

    /*!
	 * sets the group subscribe flag
	 * @param token this is set to token value associated to this request
	 * @param grpId Id of group whose subscribe file will be changed
	 * @param status
	 * @param mask
	 */
    void setGroupStatusFlags(uint32_t& token, const RsGxsGroupId& grpId, const uint32_t& status, const uint32_t& mask);

    /*!
	 * sets the group service string
	 * @param token this is set to token value associated to this request
	 * @param grpId Id of group whose subscribe file will be changed
	 * @param servString
	 */
    void setGroupServiceString(uint32_t& token, const RsGxsGroupId& grpId, const std::string& servString);

    /*!
     * sets the msg status flag
     * @param token this is set to token value associated to this request
     * @param grpId Id of group whose subscribe file will be changed
     * @param status
     * @param mask Mask to apply to status flag
     */
    void setMsgStatusFlags(uint32_t& token, const RsGxsGrpMsgIdPair& msgId, const uint32_t& status, const uint32_t& mask);

    /*!
     * sets the message service string
     * @param token this is set to token value associated to this request
     * @param msgId Id of message whose service string will be changed
     * @param servString The service string to set msg to
     */
    void setMsgServiceString(uint32_t& token, const RsGxsGrpMsgIdPair& msgId, const std::string& servString );

protected:

    /** Notifications **/

    /*!
     * This confirms this class as an abstract one that \n
     * should not be instantiated \n
     * The deriving class should implement this function \n
     * as it is called by the backend GXS system to \n
     * update client of changes which should \n
     * instigate client to retrieve new content from the system
     * Note! For newly received message and groups, bit 0xf00 is set to
     * GXS_SERV::GXS_MSG_STATUS_UNPROCESSED and GXS_SERV::GXS_MSG_STATUS_UNREAD
     * @param changes the changes that have occured to data held by this service
     */
    virtual void notifyChanges(std::vector<RsGxsNotify*>& changes) = 0;




private:

    void processRecvdData();

    void processRecvdMessages();

    void processRecvdGroups();

    void publishGrps();

    void publishMsgs();

    /*!
     * processes msg local meta changes
     */
    void processMsgMetaChanges();

    /*!
     * Processes group local meta changes
     */
    void processGrpMetaChanges();

    /*!
     * Convenience function for properly applying masks for status and subscribe flag
     * of a group.
     * @warning mask entry is removed from grpCv
     */
    bool processGrpMask(const RsGxsGroupId& grpId, ContentValue& grpCv);

    /*!
     * This completes the creation of an instance on RsNxsGrp
     * by assigning it a groupId and signature via SHA1 and EVP_sign respectively \n
     * @param grp Nxs group to create
     * @return CREATE_SUCCESS for success, CREATE_FAIL for fail,
     * 		   CREATE_FAIL_TRY_LATER for Id sign key not avail (but requested)
     */
    uint8_t createGroup(RsNxsGrp* grp, RsTlvSecurityKeySet& privateKeySet, RsTlvSecurityKeySet& publicKeySet);

    /*!
     * This completes the creation of an instance on RsNxsMsg
     * by assigning it a groupId and signature via SHA1 and EVP_sign respectively
     * What signatures are calculated are based on the authentication policy
     * of the service
     * @param msg the Nxs message to create
     * CREATE_FAIL, CREATE_SUCCESS, CREATE_ID_SIGN_NOT_AVAIL
     * @return CREATE_SUCCESS for success, CREATE_FAIL for fail,
     * 		   CREATE_FAIL_TRY_LATER for Id sign key not avail (but requested)
     */
    int createMessage(RsNxsMsg* msg);

    /*!
     * convenience function to create sign
     * @param signSet signatures are stored here
     * @param msgData message data to be signed
     * @param grpMeta the meta data for group the message belongs to
     * @return SIGN_SUCCESS for success, SIGN_FAIL for fail,
     * 		   SIGN_FAIL_TRY_LATER for Id sign key not avail (but requested), try later
     */
    int createMsgSignatures(RsTlvKeySignatureSet& signSet, RsTlvBinaryData& msgData,
                             const RsGxsMsgMetaData& msgMeta, RsGxsGrpMetaData& grpMeta);

    /*!
     * convenience function to create sign for groups
     * @param signSet signatures are stored here
     * @param grpData group data to be signed
     * @param grpMeta the meta data for group to be signed
     * @return SIGN_SUCCESS for success, SIGN_FAIL for fail,
     * 		   SIGN_FAIL_TRY_LATER for Id sign key not avail (but requested), try later
     */
    int createGroupSignatures(RsTlvKeySignatureSet& signSet, RsTlvBinaryData& grpData,
    							RsGxsGrpMetaData& grpMeta);

    /*!
     * check meta change is legal
     * @return false if meta change is not legal
     */
    bool locked_validateGrpMetaChange(GrpLocMetaData&);

    /*!
     * Generate a set of keys that can define a GXS group
     * @param privatekeySet contains private generated keys
     * @param privatekeySet contains public generated keys (counterpart of private)
     * @param genPublicKeys should publish key pair also be generated
     */
    void generateGroupKeys(RsTlvSecurityKeySet& privatekeySet, RsTlvSecurityKeySet& publickeySet, bool genPublishKeys);

    /*!
     * Attempts to validate msg signatures
     * @param msg message to be validated
     * @param grpFlag the flag for the group the message belongs to
     * @param grpKeySet the key set user has for the message's group
     * @return VALIDATE_SUCCESS for success, VALIDATE_FAIL for fail,
     * 		   VALIDATE_ID_SIGN_NOT_AVAIL for Id sign key not avail (but requested)
     */
    int validateMsg(RsNxsMsg* msg, const uint32_t& grpFlag, RsTlvSecurityKeySet& grpKeySet);

    /*!
	 * Attempts to validate group signatures
	 * @param grp group to be validated
	 * @param grpKeySet the keys set user has for the group
	 * @return VALIDATE_SUCCESS for success, VALIDATE_FAIL for fail,
	 * 		   VALIDATE_ID_SIGN_NOT_AVAIL for Id sign key not avail (but requested)
	 */
	int validateGrp(RsNxsGrp* grp, RsTlvSecurityKeySet& grpKeySet);

    /*!
     * Checks flag against a given privacy bit block
     * @param pos Determines 8 bit wide privacy block to check
     * @param flag the flag to and(&) against
     * @param the result of the (bit-block & flag)
     */
    bool checkAuthenFlag(const PrivacyBitPos& pos, const uint8_t& flag) const;

    void  groupShareKeys(std::list<std::string> peers);

private:

    RsMutex mGenMtx;
    RsGxsDataAccess* mDataAccess;
    RsGeneralDataService* mDataStore;
    RsNetworkExchangeService *mNetService;
    RsSerialType *mSerialiser;
    /// service type
    uint16_t mServType;
    RsGixs* mGixs;

    std::vector<RsNxsMsg*> mReceivedMsgs;

    typedef std::vector<GxsPendingItem<RsNxsGrp*, RsGxsGroupId> > NxsGrpPendValidVect;
    NxsGrpPendValidVect mReceivedGrps;

    std::vector<GxsGrpPendingSign> mGrpsToPublish;
    typedef std::vector<GxsGrpPendingSign> NxsGrpSignPendVect;

    std::map<uint32_t, RsGxsMsgItem*> mMsgsToPublish;

    std::map<uint32_t, RsGxsGrpMsgIdPair > mMsgNotify;
    std::map<uint32_t, RsGxsGroupId> mGrpNotify;

    // for loc meta changes
    std::map<uint32_t, GrpLocMetaData > mGrpLocMetaMap;
    std::map<uint32_t,  MsgLocMetaData> mMsgLocMetaMap;

    std::vector<RsGxsNotify*> mNotifications;



    /// authentication policy
    uint32_t mAuthenPolicy;

    std::map<uint32_t, GxsPendingItem<RsGxsMsgItem*, uint32_t> >
    	mMsgPendingSign;

    std::vector<GxsPendingItem<RsNxsMsg*, RsGxsGrpMsgIdPair> > mMsgPendingValidate;
    typedef std::vector<GxsPendingItem<RsNxsMsg*, RsGxsGrpMsgIdPair> > NxsMsgPendingVect;



private:

    std::vector<RsGxsNotify*> mChanges;
    std::vector<RsGxsGroupChange*> mGroupChange;
    std::vector<RsGxsMsgChange*> mMsgChange;

    const uint8_t CREATE_FAIL, CREATE_SUCCESS, CREATE_FAIL_TRY_LATER, SIGN_MAX_ATTEMPTS;
    const uint8_t SIGN_FAIL, SIGN_SUCCESS, SIGN_FAIL_TRY_LATER;
    const uint8_t VALIDATE_FAIL, VALIDATE_SUCCESS, VALIDATE_FAIL_TRY_LATER, VALIDATE_MAX_ATTEMPTS;
};

#endif // RSGENEXCHANGE_H
