/*
 * libretroshare/src/services: p3idservice.h
 *
 * Identity interface for RetroShare.
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

#ifndef P3_IDENTITY_SERVICE_HEADER
#define P3_IDENTITY_SERVICE_HEADER


#include "retroshare/rsidentity.h"	// External Interfaces.
#include "gxs/rsgenexchange.h"		// GXS service.
#include "gxs/rsgixs.h"			// Internal Interfaces.

#include "gxs/gxstokenqueue.h"		

#include <map>
#include <string>

#include "util/rsmemcache.h"
#include "util/rstickevent.h"

#include "pqi/authgpg.h"

/* 
 * Identity Service
 *
 */

// INTERNAL DATA TYPES. 
// Describes data stored in GroupServiceString.

class SSBit
{
	public:
virtual	bool load(const std::string &input) = 0;
virtual	std::string save() const = 0;
};



class SSGxsIdPgp: public SSBit 
{
	public:
	SSGxsIdPgp()
	:idKnown(false), lastCheckTs(0), checkAttempts(0) { return; }

virtual	bool load(const std::string &input);
virtual	std::string save() const;

	bool idKnown;
	time_t lastCheckTs;
	uint32_t checkAttempts;
	std::string pgpId;
};

class SSGxsIdScore: public SSBit 
{
	public:
	SSGxsIdScore()
	:score(0) { return; }

virtual	bool load(const std::string &input);
virtual	std::string save() const;

	int score;
};

class SSGxsIdCumulator: public SSBit
{
public:
	SSGxsIdCumulator()
	:count(0), nullcount(0), sum(0), sumsq(0) { return; }

virtual	bool load(const std::string &input);
virtual	std::string save() const;

	uint32_t count;
	uint32_t nullcount;
	double   sum;
	double   sumsq;
	
	// derived parameters:
};

class SSGxsIdGroup: public SSBit
{
public:
	SSGxsIdGroup() { return; }

virtual	bool load(const std::string &input);
virtual	std::string save() const;

	// pgphash status
	SSGxsIdPgp pgp;

	// reputation score.	
	SSGxsIdScore    score;
	SSGxsIdCumulator opinion;
	SSGxsIdCumulator reputation;
	
};

#define ID_LOCAL_STATUS_FULL_CALC_FLAG	0x00010000
#define ID_LOCAL_STATUS_INC_CALC_FLAG	0x00020000


#define MAX_CACHE_SIZE	100 // Small for testing..
//#define MAX_CACHE_SIZE	10000 // More useful size

class RsGxsIdGroupItem;

class RsGxsIdCache
{
	public:
	RsGxsIdCache();
	RsGxsIdCache(const RsGxsIdGroupItem *item, const RsTlvSecurityKey &in_pkey);

void	updateServiceString(std::string serviceString);

	RsIdentityDetails details;
	RsTlvSecurityKey pubkey;
};


#if 0
class LruData
{
	public:
	RsGxsId key;
};
#endif

	

// Not sure exactly what should be inherited here?
// Chris - please correct as necessary.

class p3IdService: public RsGxsIdExchange, public RsIdentity, 
		public GxsTokenQueue, public RsTickEvent
{
	public:
	p3IdService(RsGeneralDataService* gds, RsNetworkExchangeService* nes);
static	uint32_t idAuthenPolicy();

	virtual void service_tick(); // needed for background processing.


	/* General Interface is provided by RsIdentity / RsGxsIfaceImpl. */

	/* Data Specific Interface */

	// These are exposed via RsIdentity.
virtual bool getGroupData(const uint32_t &token, std::vector<RsGxsIdGroup> &groups);

	// These are local - and not exposed via RsIdentity.
virtual bool getMsgData(const uint32_t &token, std::vector<RsGxsIdOpinion> &opinions);
virtual bool createGroup(uint32_t& token, RsGxsIdGroup &group);
virtual bool createMsg(uint32_t& token, RsGxsIdOpinion &opinion);

	/**************** RsIdentity External Interface.
	 * Notes:
	 * 
	 * All the data is cached together for the moment - We should probably
	 * seperate and sort this out.
	 * 
	 * Also need to handle Cache updates / invalidation from internal changes.
	 * 
	 */

//virtual bool  getNickname(const RsGxsId &id, std::string &nickname);
virtual bool  getIdDetails(const RsGxsId &id, RsIdentityDetails &details);
virtual bool  getOwnIds(std::list<RsGxsId> &ownIds);



        // 
virtual bool submitOpinion(uint32_t& token, RsIdOpinion &opinion);
virtual bool createIdentity(uint32_t& token, RsIdentityParameters &params);


	/**************** RsGixs Implementation 
	 * Notes:
	 *   Interface is only suggestion at the moment, will be changed as necessary.
	 *   Results should be cached / preloaded for maximum speed.
	 *
	 */
virtual bool haveKey(const RsGxsId &id);
virtual bool requestKey(const RsGxsId &id, const std::list<PeerId> &peers);
virtual int  getKey(const RsGxsId &id, RsTlvSecurityKey &key);

virtual bool havePrivateKey(const RsGxsId &id);
virtual bool requestPrivateKey(const RsGxsId &id);
virtual int  getPrivateKey(const RsGxsId &id, RsTlvSecurityKey &key);  

	/**************** RsGixsReputation Implementation 
	 * Notes:
	 *   Again should be cached if possible.
	 */

        // get Reputation.
virtual bool getReputation(const RsGxsId &id, const GixsReputation &rep);


	protected:

	/** Notifications **/
virtual void notifyChanges(std::vector<RsGxsNotify*>& changes);

	/** Overloaded to add PgpIdHash to Group Definition **/
virtual ServiceCreate_Return service_CreateGroup(RsGxsGrpItem* grpItem, RsTlvSecurityKeySet& keySet);

        // Overloaded from GxsTokenQueue for Request callbacks.
virtual void handleResponse(uint32_t token, uint32_t req_type);

        // Overloaded from RsTickEvent.
virtual void handle_event(uint32_t event_type, const std::string &elabel);

	private:

/************************************************************************
 * This is the Cache for minimising calls to the DataStore.
 *
 */
	int  cache_tick();

	bool cache_request_load(const RsGxsId &id);
	bool cache_start_load();
	bool cache_load_for_token(uint32_t token);

	bool cache_store(const RsGxsIdGroupItem *item);
	bool cache_update_if_cached(const RsGxsId &id, std::string serviceString);

	// Mutex protected.

	std::list<RsGxsId> mCacheLoad_ToCache;

	// Switching to RsMemCache for Key Caching.
	RsMemCache<RsGxsId, RsGxsIdCache> mPublicKeyCache;
	RsMemCache<RsGxsId, RsGxsIdCache> mPrivateKeyCache;

/************************************************************************
 * Refreshing own Ids.
 *
 */
	bool cache_request_ownids();
	bool cache_load_ownids(uint32_t token);

	std::list<RsGxsId> mOwnIds;

/************************************************************************
 * Test fns for Caching.
 *
 */
	bool cachetest_tick();
	bool cachetest_getlist();
	bool cachetest_handlerequest(uint32_t token);

/************************************************************************
 * for processing background tasks that use the serviceString.
 * - must be mutually exclusive to avoid clashes.
 */
	bool CacheArbitration(uint32_t mode);
	void CacheArbitrationDone(uint32_t mode);

	bool mBgSchedule_Active;
	uint32_t mBgSchedule_Mode;

/************************************************************************
 * pgphash processing.
 *
 */
	bool pgphash_start();
	bool pgphash_handlerequest(uint32_t token);
	bool pgphash_process();

	bool checkId(const RsGxsIdGroup &grp, PGPIdType &pgp_id);
	void getPgpIdList();

	/* MUTEX PROTECTED DATA (mIdMtx - maybe should use a 2nd?) */

	std::map<PGPIdType, PGPFingerprintType> mPgpFingerprintMap;
	std::list<RsGxsIdGroup> mGroupsToProcess;

/************************************************************************
 * Below is the background task for processing opinions => reputations 
 *
 */

virtual void generateDummyData();
	void generateDummy_OwnIds();
	void generateDummy_FriendPGP();
	void generateDummy_UnknownPGP();
	void generateDummy_UnknownPseudo();

std::string genRandomId(int len = 20);

	bool reputation_start();
	bool reputation_continue();

	int	background_tick();
	bool background_checkTokenRequest();
	bool background_requestGroups();
	bool background_requestNewMessages();
	bool background_processNewMessages();
	bool background_FullCalcRequest();
	bool background_processFullCalc();
	
	bool background_cleanup();

	RsMutex mIdMtx;

	/***** below here is locked *****/
	bool mLastBgCheck;
	bool mBgProcessing;
	
	uint32_t mBgToken;
	uint32_t mBgPhase;
	
	std::map<std::string, RsGroupMetaData> mBgGroupMap;
	std::list<std::string> mBgFullCalcGroups;

/************************************************************************
 * Other Data that is protected by the Mutex.
 */

	std::vector<RsGxsGroupChange*> mGroupChange;
	std::vector<RsGxsMsgChange*> mMsgChange;

};

#endif // P3_IDENTITY_SERVICE_HEADER



