
/*
 * libretroshare/src/gxs: rsgxnetservice.cc
 *
 * Access to rs network and synchronisation service implementation
 *
 * Copyright 2012-2012 by Christopher Evi-Parker
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

#include <unistd.h>
#include <math.h>

#include "rsgxsnetservice.h"
#include "rsgxsflags.h"

#define NXS_NET_DEBUG

#define SYNC_PERIOD 12 // in microseconds every 10 seconds (1 second for testing)
#define TRANSAC_TIMEOUT 5 // 5 seconds

 const uint32_t RsGxsNetService::FRAGMENT_SIZE = 150000;

RsGxsNetService::RsGxsNetService(uint16_t servType, RsGeneralDataService *gds,
                                 RsNxsNetMgr *netMgr, RsNxsObserver *nxsObs)
                                     : p3Config(servType), p3ThreadedService(servType),
                                       mTransactionTimeOut(TRANSAC_TIMEOUT), mServType(servType), mDataStore(gds), mTransactionN(0),
                                       mObserver(nxsObs), mNxsMutex("RsGxsNetService"), mNetMgr(netMgr), mSYNC_PERIOD(SYNC_PERIOD), mSyncTs(0)

{
	addSerialType(new RsNxsSerialiser(mServType));
	mOwnId = mNetMgr->getOwnId();
}

RsGxsNetService::~RsGxsNetService()
{

}


int RsGxsNetService::tick(){

	// always check for new items arriving
	// from peers
    if(receivedItems())
        recvNxsItemQueue();

    uint32_t now = time(NULL);
    uint32_t elapsed = mSYNC_PERIOD + mSyncTs;

    if((elapsed) < now)
    {
    	syncWithPeers();
    	mSyncTs = now;
    }

    return 1;
}

void RsGxsNetService::syncWithPeers()
{

	std::set<std::string> peers;
	mNetMgr->getOnlineList(peers);

	std::set<std::string>::iterator sit = peers.begin();

	// for now just grps
	for(; sit != peers.end(); sit++)
	{
			RsNxsSyncGrp *grp = new RsNxsSyncGrp(mServType);
			grp->clear();
			grp->PeerId(*sit);
			sendItem(grp);
	}

//#ifdef GXS_ENABLE_SYNC_MSGS
        std::map<RsGxsGroupId, RsGxsGrpMetaData* > grpMeta;
        mDataStore->retrieveGxsGrpMetaData(grpMeta);

        std::map<RsGxsGroupId, RsGxsGrpMetaData* >::iterator
                mit = grpMeta.begin();

        std::vector<RsGxsGroupId> grpIds;

        for(; mit != grpMeta.end(); mit++)
        {
            RsGxsGrpMetaData* meta = mit->second;

            if(meta->mSubscribeFlags & (GXS_SERV::GROUP_SUBSCRIBE_SUBSCRIBED |
               GXS_SERV::GROUP_SUBSCRIBE_ADMIN) )
                grpIds.push_back(mit->first);

            delete meta;
        }

        sit = peers.begin();

        // synchronise group msg for groups which we're subscribed to
        for(; sit != peers.end(); sit++)
        {
            RsStackMutex stack(mNxsMutex);

            std::vector<RsGxsGroupId>::iterator vit = grpIds.begin();

            for(; vit != grpIds.end(); vit++)
            {
                    RsNxsSyncMsg* msg = new RsNxsSyncMsg(mServType);
                    msg->clear();
                    msg->PeerId(*sit);
                    msg->grpId = *vit;
                    sendItem(msg);
            }
        }
//#endif
}


bool RsGxsNetService::fragmentMsg(RsNxsMsg& msg, MsgFragments& msgFragments) const
{
	// first determine how many fragments
	uint32_t msgSize = msg.msg.TlvSize();
	uint32_t dataLeft = msgSize;
	uint8_t nFragments = ceil(float(msgSize)/FRAGMENT_SIZE);
	char buffer[FRAGMENT_SIZE];
	int currPos = 0;


	for(uint8_t i=0; i < nFragments; i++)
	{
		RsNxsMsg* msgFrag = new RsNxsMsg(mServType);
		msgFrag->grpId = msg.grpId;
		msgFrag->msgId = msg.msgId;
		msgFrag->meta = msg.meta;
		msgFrag->pos = i;
		msgFrag->count = nFragments;
		uint32_t fragSize = std::min(dataLeft, FRAGMENT_SIZE);

		memcpy(buffer, ((char*)msg.msg.bin_data) + currPos, fragSize);

		currPos += fragSize;
		dataLeft -= fragSize;
		msgFragments.push_back(msgFrag);
	}

	return true;
}

bool RsGxsNetService::fragmentGrp(RsNxsGrp& grp, GrpFragments& grpFragments) const
{
	// first determine how many fragments
	uint32_t grpSize = grp.grp.TlvSize();
	uint32_t dataLeft = grpSize;
	uint8_t nFragments = ceil(float(grpSize)/FRAGMENT_SIZE);
	char buffer[FRAGMENT_SIZE];
	int currPos = 0;


	for(uint8_t i=0; i < nFragments; i++)
	{
		RsNxsGrp* grpFrag = new RsNxsGrp(mServType);
		grpFrag->grpId = grp.grpId;
		grpFrag->meta = grp.meta;
		grpFrag->pos = i;
		grpFrag->count = nFragments;
		uint32_t fragSize = std::min(dataLeft, FRAGMENT_SIZE);

		memcpy(buffer, ((char*)grp.grp.bin_data) + currPos, fragSize);

		currPos += fragSize;
		dataLeft -= fragSize;
		grpFragments.push_back(grpFrag);
	}

	return true;
}

RsNxsMsg* RsGxsNetService::deFragmentMsg(MsgFragments& msgFragments) const
{
	if(msgFragments.empty()) return NULL;

	// first determine total size for binary data
	MsgFragments::iterator mit = msgFragments.begin();
	uint32_t datSize = 0;

	for(; mit != msgFragments.end(); mit++)
		datSize += (*mit)->msg.bin_len;

	char* data = new char[datSize];
	uint32_t currPos = 0;

	for(mit = msgFragments.begin(); mit != msgFragments.end(); mit++)
	{
		RsNxsMsg* msg = *mit;
		memcpy(data + (currPos), msg->msg.bin_data, msg->msg.bin_len);
		currPos += msg->msg.bin_len;
	}

	RsNxsMsg* msg = new RsNxsMsg(mServType);
	const RsNxsMsg& m = *(*(msgFragments.begin()));
	msg->msg.setBinData(data, datSize);
	msg->msgId = m.msgId;
	msg->grpId = m.grpId;
	msg->transactionNumber = m.transactionNumber;
	msg->meta = m.meta;

	delete[] data;
	return msg;
}

RsNxsGrp* RsGxsNetService::deFragmentGrp(GrpFragments& grpFragments) const
{
	if(grpFragments.empty()) return NULL;

	// first determine total size for binary data
	GrpFragments::iterator mit = grpFragments.begin();
	uint32_t datSize = 0;

	for(; mit != grpFragments.end(); mit++)
		datSize += (*mit)->grp.bin_len;

	char* data = new char[datSize];
	uint32_t currPos = 0;

	for(mit = grpFragments.begin(); mit != grpFragments.end(); mit++)
	{
		RsNxsGrp* grp = *mit;
		memcpy(data + (currPos), grp->grp.bin_data, grp->grp.bin_len);
		currPos += grp->grp.bin_len;
	}

	RsNxsGrp* grp = new RsNxsGrp(mServType);
	const RsNxsGrp& g = *(*(grpFragments.begin()));
	grp->grp.setBinData(data, datSize);
	grp->grpId = g.grpId;
	grp->transactionNumber = g.transactionNumber;
	grp->meta = g.meta;

	delete[] data;

	return grp;
}

struct GrpFragCollate
{
	RsGxsGroupId mGrpId;
	GrpFragCollate(const RsGxsGroupId& grpId) : mGrpId(grpId){ }
	bool operator()(RsNxsGrp* grp) { return grp->grpId == mGrpId;}
};

void RsGxsNetService::collateGrpFragments(GrpFragments fragments,
		std::map<RsGxsGroupId, GrpFragments>& partFragments) const
{
	// get all unique grpIds;
	GrpFragments::iterator vit = fragments.begin();
	std::set<RsGxsGroupId> grpIds;

	for(; vit != fragments.end(); vit++)
		grpIds.insert( (*vit)->grpId );

	std::set<RsGxsGroupId>::iterator sit = grpIds.begin();

	for(; sit != grpIds.end(); sit++)
	{
		const RsGxsGroupId& grpId = *sit;
		GrpFragments::iterator bound = std::partition(
					fragments.begin(), fragments.end(),
					GrpFragCollate(grpId));

		// something will always be found for a group id
		for(vit = fragments.begin(); vit != bound; )
		{
			partFragments[grpId].push_back(*vit);
			vit = fragments.erase(vit);
		}

		GrpFragments& f = partFragments[grpId];
		RsNxsGrp* grp = *(f.begin());

		// if counts of fragments is incorrect remove
		// from coalescion
		if(grp->count != f.size())
		{
			GrpFragments::iterator vit2 = f.begin();

			for(; vit2 != f.end(); vit2++)
				delete *vit2;

			partFragments.erase(grpId);
		}
	}

	fragments.clear();
}

struct MsgFragCollate
{
	RsGxsGroupId mMsgId;
	MsgFragCollate(const RsGxsMessageId& msgId) : mMsgId(msgId){ }
	bool operator()(RsNxsMsg* msg) { return msg->msgId == mMsgId;}
};

void RsGxsNetService::collateMsgFragments(MsgFragments fragments, std::map<RsGxsMessageId, MsgFragments>& partFragments) const
{
	// get all unique message Ids;
	MsgFragments::iterator vit = fragments.begin();
	std::set<RsGxsMessageId> msgIds;

	for(; vit != fragments.end(); vit++)
		msgIds.insert( (*vit)->msgId );


	std::set<RsGxsMessageId>::iterator sit = msgIds.begin();

	for(; sit != msgIds.end(); sit++)
	{
		const RsGxsMessageId& msgId = *sit;
		MsgFragments::iterator bound = std::partition(
					fragments.begin(), fragments.end(),
					MsgFragCollate(msgId));

		// something will always be found for a group id
		for(vit = fragments.begin(); vit != bound; )
		{
			partFragments[msgId].push_back(*vit);
			vit = fragments.erase(vit);
		}

		MsgFragments& f = partFragments[msgId];
		RsNxsMsg* msg = *(f.begin());

		// if counts of fragments is incorrect remove
		// from coalescion
		if(msg->count != f.size())
		{
			MsgFragments::iterator vit2 = f.begin();

			for(; vit2 != f.end(); vit2++)
				delete *vit2;

			partFragments.erase(msgId);
		}
	}

	fragments.clear();
}

bool RsGxsNetService::loadList(std::list<RsItem*>& load)
{
	return false;
}

bool RsGxsNetService::saveList(bool& cleanup, std::list<RsItem*>& save)
{
	return false;
}

RsSerialiser *RsGxsNetService::setupSerialiser()
{
	return NULL;
}

void RsGxsNetService::recvNxsItemQueue(){

    RsItem *item ;

    while(NULL != (item=recvItem()))
    {
#ifdef NXS_NET_DEBUG
		std::cerr << "RsGxsNetService Item:" << (void*)item << std::endl ;
#endif
		// RsNxsItem needs dynamic_cast, since they have derived siblings.
		//
		RsNxsItem *ni = dynamic_cast<RsNxsItem*>(item) ;
		if(ni != NULL)
		{

			// a live transaction has a non zero value
			if(ni->transactionNumber != 0){

#ifdef NXS_NET_DEBUG
				std::cerr << "recvNxsItemQueue()" << std::endl;
				std::cerr << "handlingTransaction, transN" << ni->transactionNumber << std::endl;
#endif

				if(handleTransaction(ni))
						continue ;
			}


			switch(ni->PacketSubType())
			{
				case RS_PKT_SUBTYPE_NXS_SYNC_GRP: handleRecvSyncGroup (dynamic_cast<RsNxsSyncGrp*>(ni)) ; break ;
				case RS_PKT_SUBTYPE_NXS_SYNC_MSG: handleRecvSyncMessage (dynamic_cast<RsNxsSyncMsg*>(ni)) ; break ;
				default:
					std::cerr << "Unhandled item subtype " << ni->PacketSubType() << " in RsGxsNetService: " << std::endl; break;
			}
			delete item ;
		}
    }
}


bool RsGxsNetService::handleTransaction(RsNxsItem* item)
{

	/*!
	 * This attempts to handle a transaction
	 * It first checks if this transaction id already exists
	 * If it does then check this not a initiating transactions
	 */

	RsStackMutex stack(mNxsMutex);

	const std::string& peer = item->PeerId();

	RsNxsTransac* transItem = dynamic_cast<RsNxsTransac*>(item);

	// if this is a RsNxsTransac item process
	if(transItem)
		return locked_processTransac(transItem);


	// then this must be transaction content to be consumed
	// first check peer exist for transaction
	bool peerTransExists = mTransactions.find(peer) != mTransactions.end();

	// then check transaction exists

	bool transExists = false;
	NxsTransaction* tr = NULL;
	uint32_t transN = item->transactionNumber;

	if(peerTransExists)
	{
		TransactionIdMap& transMap = mTransactions[peer];

		transExists = transMap.find(transN) != transMap.end();

		if(transExists)
		{

#ifdef NXS_NET_DEBUG
			std::cerr << "handleTransaction() " << std::endl;
			std::cerr << "Consuming Transaction content, transN: " << item->transactionNumber << std::endl;
			std::cerr << "Consuming Transaction content, from Peer: " << item->PeerId() << std::endl;
#endif

			tr = transMap[transN];
			tr->mItems.push_back(item);

			return true;
		}
	}

	return false;
}

bool RsGxsNetService::locked_processTransac(RsNxsTransac* item)
{

	/*!
	 * To process the transaction item
	 * It can either be initiating a transaction
	 * or ending one that already exists
	 *
	 * For initiating an incoming transaction the peer
	 * and transaction item need not exists
	 * as the peer will be added and transaction number
	 * added thereafter
	 *
	 * For commencing/starting an outgoing transaction
	 * the transaction must exist already
	 *
	 * For ending a transaction the
	 */

	std::string peer;

	// for outgoing transaction use own id
	if(item->transactFlag & (RsNxsTransac::FLAG_BEGIN_P2 | RsNxsTransac::FLAG_END_SUCCESS))
		peer = mOwnId;
	else
		peer = item->PeerId();

	uint32_t transN = item->transactionNumber;
	item->timestamp = time(NULL); // register time received
	NxsTransaction* tr = NULL;

#ifdef NXS_NET_DEBUG
	std::cerr << "locked_processTransac() " << std::endl;
	std::cerr << "locked_processTransac(), Received transaction item: " << transN << std::endl;
	std::cerr << "locked_processTransac(), With peer: " << item->PeerId() << std::endl;
	std::cerr << "locked_processTransac(), trans type: " << item->transactFlag << std::endl;
#endif

	bool peerTrExists = mTransactions.find(peer) != mTransactions.end();
	bool transExists = false;

	if(peerTrExists){

		TransactionIdMap& transMap = mTransactions[peer];
		// record whether transaction exists already
		transExists = transMap.find(transN) != transMap.end();

	}

	// initiating an incoming transaction
	if(item->transactFlag & RsNxsTransac::FLAG_BEGIN_P1){

		// create a transaction if the peer does not exist
		if(!peerTrExists){
			mTransactions[peer] = TransactionIdMap();
		}

		TransactionIdMap& transMap = mTransactions[peer];

		if(transExists)
			return false; // should not happen!

		// create new transaction
		tr = new NxsTransaction();
		transMap[transN] = tr;
		tr->mTransaction = item;
		tr->mTimeOut = item->timestamp + mTransactionTimeOut;

		// note state as receiving, commencement item
		// is sent on next run() loop
		tr->mFlag = NxsTransaction::FLAG_STATE_STARTING;

		// commencement item for outgoing transaction
	}else if(item->transactFlag & RsNxsTransac::FLAG_BEGIN_P2){

		// transaction must exist
		if(!peerTrExists || !transExists)
			return false;


		// alter state so transaction content is sent on
		// next run() loop
		TransactionIdMap& transMap = mTransactions[mOwnId];
		NxsTransaction* tr = transMap[transN];
		tr->mFlag = NxsTransaction::FLAG_STATE_SENDING;

		// end transac item for outgoing transaction
	}else if(item->transactFlag & RsNxsTransac::FLAG_END_SUCCESS){

		// transaction does not exist
		if(!peerTrExists || !transExists){
			return false;
		}

		// alter state so that transaction is removed
		// on next run() loop
		TransactionIdMap& transMap = mTransactions[mOwnId];
		NxsTransaction* tr = transMap[transN];
		tr->mFlag = NxsTransaction::FLAG_STATE_COMPLETED;
	}

	return true;
}

void RsGxsNetService::run(){


    double timeDelta = 0.2;

    while(isRunning()){

#ifndef WINDOWS_SYS
        usleep((int) (timeDelta * 1000000));
#else
        Sleep((int) (timeDelta * 1000));
#endif

        // process active transactions
        processTransactions();

        // process completed transactions
        processCompletedTransactions();

    }
}

bool RsGxsNetService::locked_checkTransacTimedOut(NxsTransaction* tr)
{
   //return tr->mTimeOut < ((uint32_t) time(NULL));
     return false;
}

void RsGxsNetService::processTransactions(){

	RsStackMutex stack(mNxsMutex);

	TransactionsPeerMap::iterator mit = mTransactions.begin();

	for(; mit != mTransactions.end(); mit++){

		TransactionIdMap& transMap = mit->second;
		TransactionIdMap::iterator mmit = transMap.begin(),

		mmit_end = transMap.end();

		/*!
		 * Transactions owned by peer
		 */
		if(mit->first == mOwnId){

			// transaction to be removed
			std::list<uint32_t> toRemove;

			for(; mmit != mmit_end; mmit++){

				NxsTransaction* tr = mmit->second;
				uint16_t flag = tr->mFlag;
				std::list<RsNxsItem*>::iterator lit, lit_end;
				uint32_t transN = tr->mTransaction->transactionNumber;

				// first check transaction has not expired
				if(locked_checkTransacTimedOut(tr))
				{

#ifdef NXS_NET_DEBUG
					std::cerr << "processTransactions() " << std::endl;
					std::cerr << "Transaction has failed, tranN: " << transN << std::endl;
					std::cerr << "Transaction has failed, Peer: " << mit->first << std::endl;
#endif

					tr->mFlag = NxsTransaction::FLAG_STATE_FAILED;
					toRemove.push_back(transN);
					continue;
				}

				// send items requested
				if(flag & NxsTransaction::FLAG_STATE_SENDING){

#ifdef NXS_NET_DEBUG
					std::cerr << "processTransactions() " << std::endl;
					std::cerr << "Sending Transaction content, transN: " << transN << std::endl;
					std::cerr << "with peer: " << tr->mTransaction->PeerId();
#endif
					lit = tr->mItems.begin();
					lit_end = tr->mItems.end();

					for(; lit != lit_end; lit++){
						sendItem(*lit);
					}

					tr->mItems.clear(); // clear so they don't get deleted in trans cleaning
					tr->mFlag = NxsTransaction::FLAG_STATE_WAITING_CONFIRM;

				}else if(flag & NxsTransaction::FLAG_STATE_WAITING_CONFIRM){
					continue;

				}else if(flag & NxsTransaction::FLAG_STATE_COMPLETED){

					// move to completed transactions
					toRemove.push_back(transN);
					mComplTransactions.push_back(tr);
				}else{

					std::cerr << "processTransactions() " << std::endl;
					std::cerr << "processTransactions(), Unknown flag for active transaction, transN: " << transN
							  << std::endl;
					std::cerr << "processTransactions(), Unknown flag, Peer: " << mit->first;
					toRemove.push_back(transN);
					tr->mFlag = NxsTransaction::FLAG_STATE_FAILED;
					mComplTransactions.push_back(tr);
				}
			}

			std::list<uint32_t>::iterator lit = toRemove.begin();

			for(; lit != toRemove.end(); lit++)
			{
				transMap.erase(*lit);
			}

		}else{

			/*!
			 * Essentially these are incoming transactions
			 * Several states are dealth with
			 * Receiving: waiting to receive items from peer's transaction
			 * and checking if all have been received
			 * Completed: remove transaction from active and tell peer
			 * involved in transaction
			 * Starting: this is a new transaction and need to teell peer
			 * involved in transaction
			 */

			std::list<uint32_t> toRemove;

			for(; mmit != mmit_end; mmit++){

				NxsTransaction* tr = mmit->second;
				uint16_t flag = tr->mFlag;
				uint32_t transN = tr->mTransaction->transactionNumber;

				// first check transaction has not expired
				if(locked_checkTransacTimedOut(tr))
				{

#ifdef NXS_NET_DEBUG
					std::cerr << "processTransactions() " << std::endl;
					std::cerr << "Transaction has failed, tranN: " << transN << std::endl;
					std::cerr << "Transaction has failed, Peer: " << mit->first << std::endl;
#endif

					tr->mFlag = NxsTransaction::FLAG_STATE_FAILED;
					toRemove.push_back(transN);
					continue;
				}

				if(flag & NxsTransaction::FLAG_STATE_RECEIVING){

					// if the number it item received equal that indicated
					// then transaction is marked as completed
					// to be moved to complete transations
					// check if done
					if(tr->mItems.size() == tr->mTransaction->nItems)
						tr->mFlag = NxsTransaction::FLAG_STATE_COMPLETED;

				}else if(flag & NxsTransaction::FLAG_STATE_COMPLETED)
				{

					// send completion msg
					RsNxsTransac* trans = new RsNxsTransac(mServType);
					trans->clear();
					trans->transactFlag = RsNxsTransac::FLAG_END_SUCCESS;
					trans->transactionNumber = transN;
					trans->PeerId(tr->mTransaction->PeerId());
					sendItem(trans);

					// move to completed transactions
					mComplTransactions.push_back(tr);

					// transaction processing done
					// for this id, add to removal list
					toRemove.push_back(mmit->first);
				}else if(flag & NxsTransaction::FLAG_STATE_STARTING){

					// send item to tell peer your are ready to start
					RsNxsTransac* trans = new RsNxsTransac(mServType);
					trans->clear();
					trans->transactFlag = RsNxsTransac::FLAG_BEGIN_P2 |
							(tr->mTransaction->transactFlag & RsNxsTransac::FLAG_TYPE_MASK);
					trans->transactionNumber = transN;
					trans->PeerId(tr->mTransaction->PeerId());
					sendItem(trans);
					tr->mFlag = NxsTransaction::FLAG_STATE_RECEIVING;

				}
				else{

					std::cerr << "processTransactions() " << std::endl;
					std::cerr << "processTransactions(), Unknown flag for active transaction, transN: " << transN
							  << std::endl;
					std::cerr << "processTransactions(), Unknown flag, Peer: " << mit->first;
					toRemove.push_back(mmit->first);
					mComplTransactions.push_back(tr);
					tr->mFlag = NxsTransaction::FLAG_STATE_FAILED; // flag as a failed transaction
				}
			}

			std::list<uint32_t>::iterator lit = toRemove.begin();

			for(; lit != toRemove.end(); lit++)
			{
				transMap.erase(*lit);
			}
		}
	}
}

void RsGxsNetService::processCompletedTransactions()
{
	RsStackMutex stack(mNxsMutex);
	/*!
	 * Depending on transaction we may have to respond to peer
	 * responsible for transaction
	 */
	std::list<NxsTransaction*>::iterator lit = mComplTransactions.begin();

	while(mComplTransactions.size()>0)
	{

		NxsTransaction* tr = mComplTransactions.front();

		bool outgoing = tr->mTransaction->PeerId() == mOwnId;

		if(outgoing){
			locked_processCompletedOutgoingTrans(tr);
		}else{
			locked_processCompletedIncomingTrans(tr);
		}


		delete tr;
		mComplTransactions.pop_front();
	}
}

void RsGxsNetService::locked_processCompletedIncomingTrans(NxsTransaction* tr)
{

	uint16_t flag = tr->mTransaction->transactFlag;

	if(tr->mFlag & NxsTransaction::FLAG_STATE_COMPLETED){
				// for a completed list response transaction
				// one needs generate requests from this
				if(flag & RsNxsTransac::FLAG_TYPE_MSG_LIST_RESP)
				{
					// generate request based on a peers response
					locked_genReqMsgTransaction(tr);

				}else if(flag & RsNxsTransac::FLAG_TYPE_GRP_LIST_RESP)
				{
					locked_genReqGrpTransaction(tr);
				}
				// you've finished receiving request information now gen
				else if(flag & RsNxsTransac::FLAG_TYPE_MSG_LIST_REQ)
				{
					locked_genSendMsgsTransaction(tr);
				}
				else if(flag & RsNxsTransac::FLAG_TYPE_GRP_LIST_REQ)
				{
					locked_genSendGrpsTransaction(tr);
				}
				else if(flag & RsNxsTransac::FLAG_TYPE_GRPS)
				{

					std::list<RsNxsItem*>::iterator lit = tr->mItems.begin();
					std::vector<RsNxsGrp*> grps;

					while(tr->mItems.size() != 0)
					{
						RsNxsGrp* grp = dynamic_cast<RsNxsGrp*>(tr->mItems.front());

						if(grp)
						{
							tr->mItems.pop_front();
							grps.push_back(grp);
						}
						else
						{
		#ifdef NXS_NET_DEBUG
							std::cerr << "RsGxsNetService::processCompletedTransactions(): item did not caste to grp"
									  << std::endl;
		#endif
						}

					}

					// notify listener of grps
					mObserver->notifyNewGroups(grps);



				}else if(flag & RsNxsTransac::FLAG_TYPE_MSGS)
				{

					std::vector<RsNxsMsg*> msgs;

					while(tr->mItems.size() > 0)
					{
						RsNxsMsg* msg = dynamic_cast<RsNxsMsg*>(tr->mItems.front());
						if(msg)
						{
							tr->mItems.pop_front();
							msgs.push_back(msg);
						}
						else
						{
		#ifdef NXS_NET_DEBUG
							std::cerr << "RsGxsNetService::processCompletedTransactions(): item did not caste to msg"
									  << std::endl;
		#endif
						}
					}

					// notify listener of msgs
					mObserver->notifyNewMessages(msgs);

				}
			}else if(tr->mFlag == NxsTransaction::FLAG_STATE_FAILED){
				// don't do anything transaction will simply be cleaned
			}
	return;
}

void RsGxsNetService::locked_processCompletedOutgoingTrans(NxsTransaction* tr)
{
	uint16_t flag = tr->mTransaction->transactFlag;

	if(tr->mFlag & NxsTransaction::FLAG_STATE_COMPLETED){
				// for a completed list response transaction
				// one needs generate requests from this
				if(flag & RsNxsTransac::FLAG_TYPE_MSG_LIST_RESP)
				{
#ifdef NXS_NET_DEBUG
					std::cerr << "processCompletedOutgoingTrans()" << std::endl;
					std::cerr << "complete Sending Msg List Response, transN: " <<
							  tr->mTransaction->transactionNumber << std::endl;
#endif
				}else if(flag & RsNxsTransac::FLAG_TYPE_GRP_LIST_RESP)
				{
#ifdef NXS_NET_DEBUG
					std::cerr << "processCompletedOutgoingTrans()" << std::endl;
					std::cerr << "complete Sending Grp Response, transN: " <<
							  tr->mTransaction->transactionNumber << std::endl;
#endif
				}
				// you've finished sending a request so don't do anything
				else if( (flag & RsNxsTransac::FLAG_TYPE_MSG_LIST_REQ) ||
						(flag & RsNxsTransac::FLAG_TYPE_GRP_LIST_REQ) )
				{
#ifdef NXS_NET_DEBUG
					std::cerr << "processCompletedOutgoingTrans()" << std::endl;
					std::cerr << "complete Sending Msg/Grp Request, transN: " <<
							  tr->mTransaction->transactionNumber << std::endl;
#endif

				}else if(flag & RsNxsTransac::FLAG_TYPE_GRPS)
				{

#ifdef NXS_NET_DEBUG
					std::cerr << "processCompletedOutgoingTrans()" << std::endl;
					std::cerr << "complete Sending Grp Data, transN: " <<
							  tr->mTransaction->transactionNumber << std::endl;
#endif
				}else if(flag & RsNxsTransac::FLAG_TYPE_MSGS)
				{
#ifdef NXS_NET_DEBUG
					std::cerr << "processCompletedOutgoingTrans()" << std::endl;
					std::cerr << "complete Sending Msg Data, transN: " <<
							  tr->mTransaction->transactionNumber << std::endl;
#endif
				}
			}else if(tr->mFlag == NxsTransaction::FLAG_STATE_FAILED){
#ifdef NXS_NET_DEBUG
				std::cerr << "processCompletedOutgoingTrans()" << std::endl;
						  std::cerr << "Failed transaction! transN: " <<
						  tr->mTransaction->transactionNumber << std::endl;
#endif
			}else{

#ifdef NXS_NET_DEBUG
					std::cerr << "processCompletedOutgoingTrans()" << std::endl;
					std::cerr << "Serious error unrecognised trans Flag! transN: " <<
							  tr->mTransaction->transactionNumber << std::endl;
#endif
			}
}


void RsGxsNetService::locked_genReqMsgTransaction(NxsTransaction* tr)
{

	// to create a transaction you need to know who you are transacting with
	// then what msgs to request
	// then add an active Transaction for request

	std::list<RsNxsSyncMsgItem*> msgItemL;

	std::list<RsNxsItem*>::iterator lit = tr->mItems.begin();

        // first get item list sent from transaction
	for(; lit != tr->mItems.end(); lit++)
	{
		RsNxsSyncMsgItem* item = dynamic_cast<RsNxsSyncMsgItem*>(*lit);
		if(item)
		{
			msgItemL.push_back(item);
		}else
		{
#ifdef NXS_NET_DEBUG
                        std::cerr << "RsGxsNetService::genReqMsgTransaction(): item failed cast to RsNxsSyncMsgItem* "
					  << std::endl;
#endif
			delete item;
			item = NULL;
		}
	}

        if(msgItemL.empty())
            return;

	// get grp id for this transaction
	RsNxsSyncMsgItem* item = msgItemL.front();
	const std::string& grpId = item->grpId;
	GxsMsgReq reqIds;
	reqIds[grpId] = std::vector<RsGxsMessageId>();
	GxsMsgMetaResult result;
	mDataStore->retrieveGxsMsgMetaData(reqIds, result);
	std::vector<RsGxsMsgMetaData*> &msgMetaV = result[grpId];

	std::vector<RsGxsMsgMetaData*>::const_iterator vit = msgMetaV.begin();
	std::set<std::string> msgIdSet;

	// put ids in set for each searching
	for(; vit != msgMetaV.end(); vit++)
		msgIdSet.insert((*vit)->mMsgId);

	// get unique id for this transaction
	uint32_t transN = locked_getTransactionId();


	// add msgs that you don't have to request list
	std::list<RsNxsSyncMsgItem*>::iterator llit = msgItemL.begin();
	std::list<RsNxsItem*> reqList;

	const std::string peerFrom = tr->mTransaction->PeerId();

	for(; llit != msgItemL.end(); llit++)
	{
		const std::string& msgId = (*llit)->msgId;

		if(msgIdSet.find(msgId) == msgIdSet.end()){
			RsNxsSyncMsgItem* msgItem = new RsNxsSyncMsgItem(mServType);
			msgItem->grpId = grpId;
			msgItem->msgId = msgId;
			msgItem->flag = RsNxsSyncMsgItem::FLAG_REQUEST;
			msgItem->transactionNumber = transN;
			msgItem->PeerId(peerFrom);
			reqList.push_back(msgItem);
		}
	}

        if(!reqList.empty())
        {

            RsNxsTransac* transac = new RsNxsTransac(mServType);
            transac->transactFlag = RsNxsTransac::FLAG_TYPE_MSG_LIST_REQ
                            | RsNxsTransac::FLAG_BEGIN_P1;
            transac->timestamp = 0;
            transac->nItems = reqList.size();
            transac->PeerId(tr->mTransaction->PeerId());
            transac->transactionNumber = transN;

            NxsTransaction* newTrans = new NxsTransaction();
            newTrans->mItems = reqList;
            newTrans->mFlag = NxsTransaction::FLAG_STATE_WAITING_CONFIRM;
            newTrans->mTimeOut = time(NULL) + mTransactionTimeOut;

            // create transaction copy with your id to indicate
            // its an outgoing transaction
            newTrans->mTransaction = new RsNxsTransac(*transac);
            newTrans->mTransaction->PeerId(mOwnId);

            sendItem(transac);

            {
                    if(!locked_addTransaction(newTrans))
                            delete newTrans;
            }
        }
}

void RsGxsNetService::locked_genReqGrpTransaction(NxsTransaction* tr)
{

	// to create a transaction you need to know who you are transacting with
	// then what grps to request
	// then add an active Transaction for request

	std::list<RsNxsSyncGrpItem*> grpItemL;

	std::list<RsNxsItem*>::iterator lit = tr->mItems.begin();

	for(; lit != tr->mItems.end(); lit++)
	{
		RsNxsSyncGrpItem* item = dynamic_cast<RsNxsSyncGrpItem*>(*lit);
		if(item)
		{
			grpItemL.push_back(item);
		}else
		{
#ifdef NXS_NET_DEBUG
			std::cerr << "RsGxsNetService::genReqMsgTransaction(): item failed to caste to RsNxsSyncMsgItem* "
					  << std::endl;
#endif
			delete item;
			item = NULL;
		}
	}

	std::map<std::string, RsGxsGrpMetaData*> grpMetaMap;
	mDataStore->retrieveGxsGrpMetaData(grpMetaMap);

	// now do compare and add loop
	std::list<RsNxsSyncGrpItem*>::iterator llit = grpItemL.begin();
	std::list<RsNxsItem*> reqList;

	uint32_t transN = locked_getTransactionId();

	for(; llit != grpItemL.end(); llit++)
	{
		const std::string& grpId = (*llit)->grpId;

		if(grpMetaMap.find(grpId) == grpMetaMap.end()){
			RsNxsSyncGrpItem* grpItem = new RsNxsSyncGrpItem(mServType);
                        grpItem->PeerId(tr->mTransaction->PeerId());
			grpItem->grpId = grpId;
			grpItem->flag = RsNxsSyncMsgItem::FLAG_REQUEST;
			grpItem->transactionNumber = transN;
			reqList.push_back(grpItem);
		}
	}


	if(!reqList.empty())
	{

		RsNxsTransac* transac = new RsNxsTransac(mServType);
		transac->transactFlag = RsNxsTransac::FLAG_TYPE_GRP_LIST_REQ
				| RsNxsTransac::FLAG_BEGIN_P1;
		transac->timestamp = 0;
		transac->nItems = reqList.size();
		transac->PeerId(tr->mTransaction->PeerId());
		transac->transactionNumber = transN;

		NxsTransaction* newTrans = new NxsTransaction();
		newTrans->mItems = reqList;
		newTrans->mFlag = NxsTransaction::FLAG_STATE_WAITING_CONFIRM;
		newTrans->mTimeOut = time(NULL) + mTransactionTimeOut;
		newTrans->mTransaction = new RsNxsTransac(*transac);
		newTrans->mTransaction->PeerId(mOwnId);

		sendItem(transac);

		if(!locked_addTransaction(newTrans))
			delete newTrans;

	}

	// clean up meta data
	std::map<std::string, RsGxsGrpMetaData*>::iterator mit = grpMetaMap.begin();

	for(; mit != grpMetaMap.end(); mit++)
		delete mit->second;
}

void RsGxsNetService::locked_genSendGrpsTransaction(NxsTransaction* tr)
{

#ifdef NXS_NET_DEBUG
	std::cerr << "locked_genSendGrpsTransaction()" << std::endl;
	std::cerr << "Generating Grp data send fron TransN: " << tr->mTransaction->transactionNumber
	          << std::endl;
#endif

	// go groups requested in transaction tr

	std::list<RsNxsItem*>::iterator lit = tr->mItems.begin();

	std::map<std::string, RsNxsGrp*> grps;

	for(;lit != tr->mItems.end(); lit++)
	{
		RsNxsSyncGrpItem* item = dynamic_cast<RsNxsSyncGrpItem*>(*lit);
		grps[item->grpId] = NULL;
	}

	if(!grps.empty())
	{
		mDataStore->retrieveNxsGrps(grps, false, false);
	}
	else{
		return;
	}

	NxsTransaction* newTr = new NxsTransaction();
	newTr->mFlag = NxsTransaction::FLAG_STATE_WAITING_CONFIRM;

	uint32_t transN = locked_getTransactionId();

	// store grp items to send in transaction
	std::map<std::string, RsNxsGrp*>::iterator mit = grps.begin();
	std::string peerId = tr->mTransaction->PeerId();
	for(;mit != grps.end(); mit++)
	{
        mit->second->PeerId(peerId); // set so it gets sent to right peer
		mit->second->transactionNumber = transN;
		newTr->mItems.push_back(mit->second);
	}

	if(newTr->mItems.empty()){
		delete newTr;
		return;
	}


	RsNxsTransac* ntr = new RsNxsTransac(mServType);
	ntr->transactionNumber = transN;
	ntr->transactFlag = RsNxsTransac::FLAG_BEGIN_P1 |
			RsNxsTransac::FLAG_TYPE_GRPS;
	ntr->nItems = grps.size();
        ntr->PeerId(tr->mTransaction->PeerId());

	newTr->mTransaction = new RsNxsTransac(*ntr);
	newTr->mTransaction->PeerId(mOwnId);
	newTr->mTimeOut = time(NULL) + mTransactionTimeOut;

	ntr->PeerId(tr->mTransaction->PeerId());
	sendItem(ntr);

	locked_addTransaction(newTr);

	return;
}

void RsGxsNetService::locked_genSendMsgsTransaction(NxsTransaction* tr)
{

#ifdef NXS_NET_DEBUG
	std::cerr << "locked_genSendMsgsTransaction()" << std::endl;
	std::cerr << "Generating Msg data send fron TransN: " << tr->mTransaction->transactionNumber
	          << std::endl;
#endif

	// go groups requested in transaction tr

	std::list<RsNxsItem*>::iterator lit = tr->mItems.begin();

	GxsMsgReq msgIds;
	GxsMsgResult msgs;

	if(tr->mItems.empty()){
		return;
	}

	for(;lit != tr->mItems.end(); lit++)
	{
		RsNxsSyncMsgItem* item = dynamic_cast<RsNxsSyncMsgItem*>(*lit);
		msgIds[item->grpId].push_back(item->msgId);
	}

	mDataStore->retrieveNxsMsgs(msgIds, msgs, false, false);

	NxsTransaction* newTr = new NxsTransaction();
	newTr->mFlag = NxsTransaction::FLAG_STATE_WAITING_CONFIRM;

	uint32_t transN = locked_getTransactionId();

	// store msg items to send in transaction
	GxsMsgResult::iterator mit = msgs.begin();
	std::string peerId = tr->mTransaction->PeerId();
	uint32_t msgSize = 0;

	for(;mit != msgs.end(); mit++)
	{
		std::vector<RsNxsMsg*>& msgV = mit->second;
		std::vector<RsNxsMsg*>::iterator vit = msgV.begin();

		for(; vit != msgV.end(); vit++)
		{
			RsNxsMsg* msg = *vit;
			msg->PeerId(peerId);
			msg->transactionNumber = transN;
			newTr->mItems.push_back(msg);
			msgSize++;
		}
	}

	if(newTr->mItems.empty()){
		delete newTr;
		return;
	}

	RsNxsTransac* ntr = new RsNxsTransac(mServType);
	ntr->transactionNumber = transN;
	ntr->transactFlag = RsNxsTransac::FLAG_BEGIN_P1 |
			RsNxsTransac::FLAG_TYPE_MSGS;
	ntr->nItems = msgSize;
	ntr->PeerId(peerId);

	newTr->mTransaction = new RsNxsTransac(*ntr);
	newTr->mTransaction->PeerId(mOwnId);
	newTr->mTimeOut = time(NULL) + mTransactionTimeOut;

	ntr->PeerId(tr->mTransaction->PeerId());
	sendItem(ntr);

	locked_addTransaction(newTr);

	return;
}
uint32_t RsGxsNetService::locked_getTransactionId()
{
	return ++mTransactionN;
}
bool RsGxsNetService::locked_addTransaction(NxsTransaction* tr)
{
	const std::string& peer = tr->mTransaction->PeerId();
	uint32_t transN = tr->mTransaction->transactionNumber;
	TransactionIdMap& transMap = mTransactions[peer];
	bool transNumExist = transMap.find(transN)
			!= transMap.end();


	if(transNumExist){
#ifdef NXS_NET_DEBUG
		std::cerr << "locked_addTransaction() " << std::endl;
		std::cerr << "Transaction number exist already, transN: " << transN
				  << std::endl;
#endif
		return false;
	}else{
		transMap[transN] = tr;
		return true;
	}
}

void RsGxsNetService::cleanTransactionItems(NxsTransaction* tr) const
{
	std::list<RsNxsItem*>::iterator lit = tr->mItems.begin();

	for(; lit != tr->mItems.end(); lit++)
	{
		delete *lit;
	}

	tr->mItems.clear();
}

void RsGxsNetService::handleRecvSyncGroup(RsNxsSyncGrp* item)
{

	RsStackMutex stack(mNxsMutex);

	std::string peer = item->PeerId();

	std::map<std::string, RsGxsGrpMetaData*> grp;
	mDataStore->retrieveGxsGrpMetaData(grp);

	if(grp.empty())
		return;

	std::map<std::string, RsGxsGrpMetaData*>::iterator mit =
	grp.begin();

	NxsTransaction* tr = new NxsTransaction();
	std::list<RsNxsItem*>& itemL = tr->mItems;

	uint32_t transN = locked_getTransactionId();

	for(; mit != grp.end(); mit++)
	{
		RsNxsSyncGrpItem* gItem = new
				RsNxsSyncGrpItem(mServType);
		gItem->flag = RsNxsSyncGrpItem::FLAG_RESPONSE;
		gItem->grpId = mit->first;
		gItem->publishTs = mit->second->mPublishTs;
		gItem->PeerId(peer);
		gItem->transactionNumber = transN;
		itemL.push_back(gItem);
		delete mit->second; // release resource
	}

	tr->mFlag = NxsTransaction::FLAG_STATE_WAITING_CONFIRM;
	RsNxsTransac* trItem = new RsNxsTransac(mServType);
	trItem->transactFlag = RsNxsTransac::FLAG_BEGIN_P1
			| RsNxsTransac::FLAG_TYPE_GRP_LIST_RESP;
	trItem->nItems = itemL.size();

	trItem->timestamp = 0;
	trItem->PeerId(peer);
	trItem->transactionNumber = transN;

	// also make a copy for the resident transaction
	tr->mTransaction = new RsNxsTransac(*trItem);
	tr->mTransaction->PeerId(mOwnId);
	tr->mTimeOut = time(NULL) + mTransactionTimeOut;

	// signal peer to prepare for transaction
	sendItem(trItem);

	locked_addTransaction(tr);

	return;
}

void RsGxsNetService::handleRecvSyncMessage(RsNxsSyncMsg* item)
{
	RsStackMutex stack(mNxsMutex);

	const std::string& peer = item->PeerId();

	GxsMsgMetaResult metaResult;
	GxsMsgReq req;
	req[item->grpId] = std::vector<std::string>();
	mDataStore->retrieveGxsMsgMetaData(req, metaResult);

	std::vector<RsGxsMsgMetaData*>& msgMeta = metaResult[item->grpId];

	if(req.empty()){
		return;
	}

	std::vector<RsGxsMsgMetaData*>::iterator vit = msgMeta.begin();

	NxsTransaction* tr = new NxsTransaction();
	std::list<RsNxsItem*>& itemL = tr->mItems;

	uint32_t transN = locked_getTransactionId();

	for(; vit != msgMeta.end(); vit++)
	{
		RsGxsMsgMetaData* m = *vit;
		RsNxsSyncMsgItem* mItem = new
				RsNxsSyncMsgItem(mServType);
		mItem->flag = RsNxsSyncGrpItem::FLAG_RESPONSE;
		mItem->grpId = m->mGroupId;
		mItem->msgId = m->mMsgId;
		mItem->PeerId(peer);
		mItem->transactionNumber = transN;
		itemL.push_back(mItem);
	}

	tr->mFlag = NxsTransaction::FLAG_STATE_WAITING_CONFIRM;
	RsNxsTransac* trItem = new RsNxsTransac(mServType);
	trItem->transactFlag = RsNxsTransac::FLAG_BEGIN_P1
			| RsNxsTransac::FLAG_TYPE_MSG_LIST_RESP;

	trItem->nItems = itemL.size();

	trItem->timestamp = 0;
	trItem->PeerId(peer);
	trItem->transactionNumber = transN;

	// also make a copy for the resident transaction
	tr->mTransaction = new RsNxsTransac(*trItem);
	tr->mTransaction->PeerId(mOwnId);
	tr->mTimeOut = time(NULL) + mTransactionTimeOut;

	// signal peer to prepare for transaction
	sendItem(trItem);

	locked_addTransaction(tr);

	return;
}


/** inherited methods **/

void RsGxsNetService::pauseSynchronisation(bool enabled)
{

}

void RsGxsNetService::setSyncAge(uint32_t age)
{

}

/** NxsTransaction definition **/

const uint8_t NxsTransaction::FLAG_STATE_STARTING = 0x0001; // when
const uint8_t NxsTransaction::FLAG_STATE_RECEIVING = 0x0002; // begin receiving items for incoming trans
const uint8_t NxsTransaction::FLAG_STATE_SENDING = 0x0004; // begin sending items for outgoing trans
const uint8_t NxsTransaction::FLAG_STATE_COMPLETED = 0x008;
const uint8_t NxsTransaction::FLAG_STATE_FAILED = 0x0010;
const uint8_t NxsTransaction::FLAG_STATE_WAITING_CONFIRM = 0x0020;


NxsTransaction::NxsTransaction()
    : mFlag(0), mTimeOut(0), mTransaction(NULL) {

}

NxsTransaction::~NxsTransaction(){

	std::list<RsNxsItem*>::iterator lit = mItems.begin();

	for(; lit != mItems.end(); lit++)
	{
		delete *lit;
		*lit = NULL;
	}

	delete mTransaction;
	mTransaction = NULL;
}


/* Net Manager */

RsNxsNetMgrImpl::RsNxsNetMgrImpl(p3LinkMgr *lMgr)
    : mLinkMgr(lMgr), mNxsNetMgrMtx("RsNxsNetMgrImpl")
{

}


std::string RsNxsNetMgrImpl::getOwnId()
{
    RsStackMutex stack(mNxsNetMgrMtx);
    return mLinkMgr->getOwnId();
}

void RsNxsNetMgrImpl::getOnlineList(std::set<std::string> &ssl_peers)
{
    ssl_peers.clear();

    std::list<std::string> pList;
    {
        RsStackMutex stack(mNxsNetMgrMtx);
        mLinkMgr->getOnlineList(pList);
    }

    std::list<std::string>::const_iterator lit = pList.begin();

    for(; lit != pList.end(); lit++)
        ssl_peers.insert(*lit);
}





