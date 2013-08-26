#ifndef BITDHT_HISTORY_H
#define BITDHT_HISTORY_H

#include "bitdht/bdpeer.h"
#include "bitdht/bdobj.h"
#include "bitdht/bdstddht.h"
#include <map>

#define MSG_TYPE_DIRECTION_MASK	0x000f0000

#define MSG_DIRECTION_INCOMING  0x00010000
#define MSG_DIRECTION_OUTGOING  0x00020000

/**** DEBUGGING HISTORY ****/

class MsgRegister
{
        public:
        MsgRegister() { return; }
        MsgRegister(const bdId *inId, uint32_t inMsgType, bool inIncoming, const bdNodeId *inAboutId)
          :id(*inId), msgType(inMsgType), incoming(inIncoming) 
	{ 
		if (inAboutId)
		{
			aboutId = *inAboutId;
		}
		else
		{
			bdStdZeroNodeId(&aboutId);
		}

		return; 
	}

        bdId id;
        uint32_t msgType;
        bool incoming;
	bdNodeId aboutId; // filled in for queries.
};


class bdMsgHistoryItem
{
	public:
	bdMsgHistoryItem() 
          :msgType(0), incoming(false) 
	{
		bdStdZeroNodeId(&aboutId);
		return;
	}

        bdMsgHistoryItem(uint32_t inMsgType, bool inIncoming, const bdNodeId *inAboutId)
          :msgType(inMsgType), incoming(inIncoming) 
	{ 
		if (inAboutId)
		{
			aboutId = *inAboutId;
		}
		else
		{
			bdStdZeroNodeId(&aboutId);
		}

		return; 
	}

	uint32_t msgType;
	bool incoming;
	bdNodeId aboutId; // filled in for queries.
};


class bdMsgHistoryList
{
	public:
	bdMsgHistoryList();
void	addMsg(time_t ts, uint32_t msgType, bool incoming, const bdNodeId *aboutId);
void 	setPeerType(time_t ts, std::string version);
int	msgCount(time_t start_ts, time_t end_ts);
bool    msgClear(time_t before); // 0 => clear all.
void	msgClear();
void	printHistory(std::ostream &out, int mode, time_t start_ts, time_t end_ts);
bool    analysePeer();
void    clearHistory();

bool    canSend();
bool    validPeer();

	std::multimap<time_t, bdMsgHistoryItem> msgHistory;
	std::string mPeerVersion;
	bdId mId;
};



class bdHistory
{
	public:
	bdHistory(time_t store_period);

void	addMsg(const bdId *id, bdToken *transId, uint32_t msgType, bool incoming, const bdNodeId *aboutId);
void 	setPeerType(const bdId *id, std::string version);
void	printMsgs();

void    cleanupOldMsgs();
void    clearHistory();
bool    analysePeers();
bool 	peerTypeAnalysis();

bool    canSend(const bdId *id);
bool    validPeer(const bdId *id);

	/* recent history */
	//std::list<bdId> lastMsgs;
	std::map<bdId, bdMsgHistoryList> mHistory;
	std::multimap<time_t, MsgRegister> mMsgTimeline;

	int mStorePeriod;

};



#endif

