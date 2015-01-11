/*
 * libretroshare/src/services: p3grouter.cc
 *
 * Services for RetroShare.
 *
 * Copyright 2013 by Cyril Soler
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
 * Please report all bugs and problems to "csoler@users.sourceforge.net".
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Decentralized routing
// =====================
// 
// Use cases:
//    - Peer A asks for B's key, for which he has the signature, or the ID.
//    - Peer A wants to send a private msg to peer C, for which he has the public key
//    - Peer A wants to contact a channel's owner, a group owner, a forum owner, etc.
//    - Peer C needs to route msg/key requests from unknown peer, to unknown peer so that the information
//       eventually reach their destination.
// 
// Main idea: Each peer holds a local routing table, a matrix with probabilities that each friend
// is a correct path for a given key ID.
// 
// The routing tables are updated as messages go back and forth. Successful
// interactions feed the routing table with information of where to route the
// packets.
// 
// The routing is kept probabilistic, meaning that the optimal route is not
// always chosen, but the randomness helps updating the routing probabilities.
// 
// Services that might use the router (All services really...)
//     - Identity manager (p3Identity)
//        - asks identities i.e. RSA public keys (i.e. sends dentity requests through router)
//     - Messenger
//        - sends/receives messages to distant peers
//     - Channels, forums, posted, etc.
//        - send messages to the origin of the channel/forum/posted
// 
// GUI
//    - a debug panel should show the routing info: probabilities for all known IDs
//    - routing probabilities for a given ID accordign to who's connected
// 
// Decentralized routing algorithm:
//    - tick() method
//       * calls autoWash(), send() and receive()
// 
//    - message passing
//       - upward: 
//          * Forward msg to friends according to probabilities.
//          * If all equal, send to all friends (or a rando subset of them).
//          * keep the local routing info in a cache that is saved (Which peer issued the msg)
//             - which probability was used to chose this friend (will be useful
//               to compute the routing contribution if the msg is ACK-ed)
//
//          Two probabilities are computed:
//          	- routing probabilities among connected friends
//          		* this is computed by the routing matrix
//          	- branching factor N
//          		* depends on the depth of the items
//          		* depends on the distribution of probabilities (min and max)
//
//          Once computed, 
//          	- the item is forwarded randomly to N peers drawn from the list of connected peers with the given probabilities.
//          	- the depth of the item is incremented randomly
// 
//       - downward: look into routing cache. If info not present, drop the item.
//         Forward item into stored direction.
// 
//       - routing probability computation: count number of times a reliable info is obtained from
//         which direction for which identity
//          * the count is a floating point number, since weights can be assigned to each info
//            (especially for importance sampling)
//          * init: all friends have equal count of 0 (or 1, well, we'll have to make this right).
//          * We use importance sampling, meaning that when peer relays a msg from ID:
//                 count[ID, peer] += 1.0 / importance
// 
//              ... where importance was the probability of chosing peer for the
//              route upward.
// 
//          * probability of forward is proportional to count.
// 
//    - routing cache
//       * this cache stores messages IDs (like turtle router) but is saved on disk
//       * it is used to remember where to send back responses to messages, and
//         with what probability the route was chosen.
//       * cache items have a TTL and the cache is cleaned regularly.
// 
//    - routing matrix
//       * the structure is fed by other services, when they receive key IDs.
//       * stores for each identity the count of how many times each peer gave reliable info for that ID.
//        That information should be enough to route packets in the correct direction. 
//       * saved to disk.
//       * all contributions should have a time stamp. Regularly, the oldest contributions are removed.
// 
//          struct RoutingMatrixHitEntry
//          {
//             float weight ;
//             time_t time_stamp ;
//          }
//          typedef std::map<std::string,std::list<RoutingMatrixHitEntry> > RSAKeyRoutingMap ;
// 
//          class RoutingMatrix
//          {
//             public:
//                // Computes the routing probabilities for this id  for the given list of friends.
//                // the computation accounts for the time at which the info was received and the
//                // weight of each routing hit record.
//                //
//                bool computeRoutingProbabilities(RSAKeyIDType id, const std::vector<SSLIdType>& friends,
//                                                 std::vector<float>& probas) const ;
// 
//                // remove oldest entries.
//                bool autoWash() ;
// 
//                // Record one routing clue. The events can possibly be merged in time buckets.
//                //
//                bool addRoutingEvent(RSAKeyIDType id,const SSLIdType& which friend) ;
// 
//             private:
//                std::map<RSAKeyIDType, RSAKeyRoutingMap> _known_keys ;
//          };
// 
//    - Routed packets: we use a common packet type for all services:
// 
//       We need two abstract item types:
// 
//          * Data packet
//             - packet unique ID (sha1, or uint64_t)
//             - destination ID (for Dn packets, the destination is the source!)
//             - packet type: Id request, Message, etc.
//             - packet service ID (Can be messenging, channels, etc).
//             - packet data (void* + size_t)
//             - flags (such as ACK or response required, and packet direction)
//             - routed directions and probabilities
//          * ACK packet.
//             - packet unique ID (the id of the corresponding data)
//             - flags (reason for ACK. Could be data delivered, or error, too far, etc)
// 
//    - Data storage packets
//       * We need storage packets for the matrix states.
//       * General routing options info?
// 
//    - estimated memory cost
//       For each identity, the matrix needs
//          - hits for each friend peer with time stamps. That means 8 bytes per hit.
//             That is for 1000 identities, having at most 100 hits each (We keep
//             the hits below a maximum. 100 seems ok.), that is 1000*100*8 < 1MB. Not much.
// 
//    - Main difficulties:
//       * have a good re-try strategy if a msg does not arrive.
//       * handle peer availability. In forward mode: easy. In backward mode:
//         difficult. We should wait, and send back the packet if possible.
//       * robustness
//       * security: avoid flooding, and message alteration.
// 
// 	- Questions to be solved
// 		* how do we talk to other services?
// 			- keep a list of services?
// 
// 			- in practice, services will need to send requests, and expect responses.
// 				* gxs (p3identity) asks for a key, gxs (p3identity) should get the key.
// 				* msg service wants to send a distant msg, or msg receives a distant msg.
// 
// 				=> we need abstract packets and service ids.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <math.h>

#include "util/rsrandom.h"
#include "util/rsprint.h"
#include "serialiser/rsconfigitems.h"
#include "services/p3idservice.h"
#include "gxs/gxssecurity.h"
#include "turtle/p3turtle.h"

#include "p3grouter.h"
#include "grouteritems.h"
#include "groutertypes.h"
#include "grouterclientservice.h"

/**********************/
//#define GROUTER_DEBUG
/**********************/
#define GROUTER_DEBUG
#define NOT_IMPLEMENTED std::cerr << __PRETTY_FUNCTION__ << ": not implemented!" << std::endl;

static const uint32_t MAX_TUNNEL_WAIT_TIME = 60 ; // wait for 60 seconds at most for a tunnel response.

const std::string p3GRouter::SERVICE_INFO_APP_NAME = "Global Router" ;

p3GRouter::p3GRouter(p3ServiceControl *sc, p3IdService *is)
    : p3Service(), p3Config(), mServiceControl(sc), mIdService(is), grMtx("GRouter")
{
	addSerialType(new RsGRouterSerialiser()) ;

	_last_autowash_time = 0 ;
	_last_debug_output_time = 0 ;
	_last_config_changed = 0 ;
	_last_matrix_update_time = 0 ;
	_debug_enabled = true ;

	_random_salt = RSRandom::random_u64() ;

	_changed = false ;
}

int p3GRouter::tick()
{
	time_t now = time(NULL) ;

	if(now > _last_autowash_time + RS_GROUTER_AUTOWASH_PERIOD)
	{
		// route pending objects
		//
		routePendingObjects() ;

		_last_autowash_time = now ;
		autoWash() ;
    }
    // Go through the list of active tunnel requests and pending objects to ask for new tunnels
    // or close existing tunnel requests.
    //
    handleTunnels() ;

    // Handle incoming items. Data comes out of tunnels.
	// 
	handleIncoming() ;
	
	// Update routing matrix
	//
	if(now > _last_matrix_update_time + RS_GROUTER_MATRIX_UPDATE_PERIOD)
	{
		RsStackMutex mtx(grMtx) ;

		_last_matrix_update_time = now ;
		_routing_matrix.updateRoutingProbabilities() ;		// This should be locked.
	}

#ifdef GROUTER_DEBUG
	// Debug dump everything
	//
	if(now > _last_debug_output_time + RS_GROUTER_DEBUG_OUTPUT_PERIOD)
	{
		_last_debug_output_time = now ;
		if(_debug_enabled)
			debugDump() ;
	}
#endif

	// If content has changed, save config, at most every RS_GROUTER_MIN_CONFIG_SAVE_PERIOD seconds appart
	// Otherwise, always save at least every RS_GROUTER_MAX_CONFIG_SAVE_PERIOD seconds
	//
	if(_changed && now > _last_config_changed + RS_GROUTER_MIN_CONFIG_SAVE_PERIOD)
	{
#ifdef GROUTER_DEBUG
		grouter_debug() << "p3GRouter::tick(): triggering config save." << std::endl;
#endif

		_changed = false ;
		_last_config_changed = now ;
		IndicateConfigChanged() ;
	}

	return 0 ;
}

time_t p3GRouter::computeNextTimeDelay(time_t stored_time)
{
	// Computes the time to wait before re-sending the object, based on how long it has been stored already.
	
	if(stored_time <       2*60  )  return      10 ;	// re-schedule every 10 secs for items not older than 2 mins. This ensures a rapid spread when peers are online.
	if(stored_time <      40*60  )  return 10 * 60 ;	// then, try every 10 mins for 40 mins
	if(stored_time <       4*3600)  return    3600 ;	// then, try every hour for 4 hours
	if(stored_time <   10*24*3600)  return 12*3600 ;	// then, try every 12 hours for 10 days
	if(stored_time < 6*30*24*3600)  return 5*86400 ;	// then, try every 5 days for 6 months

	return 6*30*86400 ;											// default: try every 5 months
}

RsSerialiser *p3GRouter::setupSerialiser()
{
	RsSerialiser *rss = new RsSerialiser ;

	rss->addSerialType(new RsGRouterSerialiser) ;
	rss->addSerialType(new RsGeneralConfigSerialiser());

	return rss ;
}

void p3GRouter::autoWash()
{
	RsStackMutex mtx(grMtx) ;

#ifdef GROUTER_DEBUG
	grouter_debug() << "p3GRouter::autoWash(): cleaning old entried." << std::endl;
#endif

	// cleanup cache

	time_t now = time(NULL) ;

    for(std::map<GRouterMsgPropagationId,GRouterRoutingInfo>::iterator it(_pending_messages.begin());it!=_pending_messages.end();)
        if(it->second.received_time_TS + GROUTER_ITEM_MAX_CACHE_KEEP_TIME < now)	// is the item too old for cache
        {
#ifdef GROUTER_DEBUG
            grouter_debug() << "  Removing cached item " << std::hex << it->first << std::dec << std::endl;
#endif
			delete it->second.data_item ;
            std::map<GRouterMsgPropagationId,GRouterRoutingInfo>::iterator tmp(it) ;
            ++tmp ;
            _pending_messages.erase(it) ;
            it = tmp ;
        }
        else
			++it ;

	// look into pending items.
	
#ifdef GROUTER_DEBUG
	grouter_debug() << "  Pending messages to route  : " << _pending_messages.size() << std::endl;
#endif
}

bool p3GRouter::registerKey(const RsGxsId& authentication_key,const GRouterServiceId& client_id,const std::string& description)
{
	RsStackMutex mtx(grMtx) ;

	if(_registered_services.find(client_id) == _registered_services.end())
	{
        std::cerr << __PRETTY_FUNCTION__ << ": unable to register key " << authentication_key << " for client id " << client_id << ": client id  is not known." << std::endl;
		return false ;
	}

	GRouterPublishedKeyInfo info ;
	info.service_id = client_id ;
    info.authentication_key = authentication_key ;
    info.description_string = description.substr(0,20);

    Sha1CheckSum hash = makeTunnelHash(authentication_key,client_id) ;

    _owned_key_ids[hash] = info ;
#ifdef GROUTER_DEBUG
	grouter_debug() << "Registered the following key: " << std::endl;
    grouter_debug() << "   Auth GXS Id : " << authentication_key << std::endl;
	grouter_debug() << "   Client id   : " << std::hex << client_id << std::dec << std::endl;
	grouter_debug() << "   Description : " << info.description_string << std::endl;
#endif

	return true ;
}
bool p3GRouter::unregisterKey(const RsGxsId& key_id,const GRouterServiceId& sid)
{
	RsStackMutex mtx(grMtx) ;

    Sha1CheckSum hash = makeTunnelHash(key_id,sid) ;

    std::map<Sha1CheckSum,GRouterPublishedKeyInfo>::iterator it = _owned_key_ids.find(hash) ;

	if(it == _owned_key_ids.end())
	{
        std::cerr << "p3GRouter::unregisterKey(): key " << key_id << " not found." << std::endl;
		return false ;
	}

#ifdef GROUTER_DEBUG
	grouter_debug() << "p3GRouter::unregistered the following key: " << std::endl;
    grouter_debug() << "   Key id      : " << key_id.toStdString() << std::endl;
	grouter_debug() << "   Client id   : " << std::hex << it->second.service_id << std::dec << std::endl;
	grouter_debug() << "   Description : " << it->second.description_string << std::endl;
#endif

	_owned_key_ids.erase(it) ;

	return true ;
}
//===========================================================================================================================//
//                                                    Turtle management                                                      //
//===========================================================================================================================//

bool p3GRouter::handleTunnelRequest(const RsFileHash& /*hash*/,const RsPeerId& /*peer_id*/)
{
    NOT_IMPLEMENTED;

    // tunnel request is answered according to the following rules:
    // 	- we are the destination => always accept
    //	- we know the destination and have RCPT items to send back => always accept
    //	- we know the destination and have a route (according to matrix) => accept with high probability
    //	- we don't know the destination => accept with very low probability

    return false ;
}
void p3GRouter::receiveTurtleData(RsTurtleGenericTunnelItem */*item*/,const RsFileHash& hash,const RsPeerId& virtual_peer_id,RsTurtleGenericTunnelItem::Direction direction)
{
    NOT_IMPLEMENTED;

    std::cerr << "p3GRouter::receiveTurtleData() " << std::endl;
    std::cerr << "  Received data for hash : " << hash << std::endl;
    std::cerr << "  Virtual peer id        : " << virtual_peer_id << std::endl;
    std::cerr << "  Direction              : " << direction << std::endl;

    // turtle data is received.
    // This function
    // 	- converts it into a grouter generic item (by deserialising it)
    //	-
}
void p3GRouter::addVirtualPeer(const TurtleFileHash& hash,const TurtleVirtualPeerId& virtual_peer_id,RsTurtleGenericTunnelItem::Direction dir)
{
    // Server side tunnels. This is incoming data. Nothing to do.

    std::cerr << "p3GRouter::addVirtualPeer(). Received vpid " << virtual_peer_id << " for hash " << hash << ", direction=" << dir << std::endl;

    std::cerr << "  adding server VPID." << std::endl;

    _virtual_peers[hash].addVirtualPeer(virtual_peer_id) ;

    if(dir == RsTurtleGenericTunnelItem::DIRECTION_SERVER)
    {
    }

    // Client side tunnels. This is outgoing data. Nothing to do.

    if(dir == RsTurtleGenericTunnelItem::DIRECTION_CLIENT)
    {
    }
}
void p3GRouter::removeVirtualPeer(const TurtleFileHash& hash,const TurtleVirtualPeerId& virtual_peer_id)
{
    NOT_IMPLEMENTED;

    // this is mostly for unused tunnels. So no real work is needed here. Just remove the tunnels from client/server lists.
}
void p3GRouter::connectToTurtleRouter(p3turtle *pt)
{
    mTurtle = pt ;
}

//===========================================================================================================================//
//                                                    Tunnel management                                                      //
//===========================================================================================================================//

// Each message is associated to a given GXS id.
//	-> messages have a state about being sent/partially arrived/etc

// Each GXS id + service might have a collection of virtual peers
// 	-> each hash has possibly multiple virtual peers associated to it.



template<class T>
static bool operator<(const std::pair<int,T>& p1,const std::pair<int,T>& p2)
{
    return p1.first < p2.first ;
}

void p3GRouter::handleTunnels()
{
    // Go through the list of pending messages
    // - if tunnels are pending for too long   =>   remove from turtle
    // - if item is waiting for too long       => tunnels are waitin

    // We need a priority queue of items to handle, starting from the most ancient items, with a delay that varies with
    // how much time they have been waiting. When a turtle slot it freed, we take the next item in the queue and
    // activate tunnel handling for it.

    // possible pending message status:
    //	- RS_GROUTER_PENDING_MSG_STATUS_TUNNEL_READY	: tunnel is ready. Waiting a few seconds to be used (this is to allow multiple tunnels to come).
    //	- RS_GROUTER_PENDING_MSG_STATUS_TUNNEL_SEND_OK  : tunnel is ready to be used
    //	- RS_GROUTER_PENDING_MSG_STATUS_TUNNEL_PENDING	: tunnel was asked.
    //	- RS_GROUTER_PENDING_MSG_STATUS_TUNNEL_UNMANAGED: not tunnel managed at the moment.

    // 1 - make a priority list of messages to ask tunnels for

    // compute the priority of pending messages, according to the number of attempts and how far in the past they have been tried for the last time.

    // Delay after which a message is re-sent, depending on the number of attempts already made.

if(!_pending_messages.empty())
{
    grouter_debug() << "p3GRouter::handleTunnels()" << std::endl;
    grouter_debug() << "  building priority list of items to send..." << std::endl;
}

    static uint32_t send_retry_time_delays[6] = { 0, 1800, 3600, 5*3600, 12*3600, 24*2600 } ;
    time_t now = time(NULL) ;
    std::vector<std::pair<int,GRouterRoutingInfo*> > priority_list ;

    for(std::map<GRouterMsgPropagationId, GRouterRoutingInfo>::iterator it=_pending_messages.begin();it!=_pending_messages.end();++it)
    {
        grouter_debug() << "    " << std::hex << it->first << std::dec << " data_status=" << it->second.data_status << ", tunnel_status=" << it->second.tunnel_status;

        if(it->second.data_status == RS_GROUTER_DATA_STATUS_PENDING)
        {
            if(it->second.tunnel_status == RS_GROUTER_TUNNEL_STATUS_UNMANAGED)
            {
                uint32_t item_delay = now - it->second.last_tunnel_request_TS ;
                int item_priority = item_delay - send_retry_time_delays[std::min(5u,it->second.sending_attempts)] ;

                grouter_debug() << "  delay=" << item_delay << " attempts=" << it->second.sending_attempts << ", priority=" << item_priority ;

                if(item_priority > 0)
                    priority_list.push_back(std::make_pair(item_priority,&it->second)) ;
            }
            else if(it->second.tunnel_status == RS_GROUTER_TUNNEL_STATUS_PENDING && it->second.last_tunnel_request_TS + MAX_TUNNEL_WAIT_TIME < now)
            {
                mTurtle->stopMonitoringTunnels(it->second.tunnel_hash) ;

                it->second.tunnel_status = RS_GROUTER_TUNNEL_STATUS_UNMANAGED ;

                grouter_debug() << "  stopping tunnels for this message." ;
            }
            else
                grouter_debug() << "  doing nothing." << std::endl;

            grouter_debug() << std::endl;
        }
    }
grouter_debug() << "  sorting..." << std::endl;

    std::sort(priority_list.begin(),priority_list.end()) ;

    // take tunnels from item priority list, and enable tunnel handling, while respecting max number of active tunnels limit

    for(uint32_t i=0;i<priority_list.size();++i)
    {
grouter_debug() << "  askign tunnel management for msg=" << priority_list[i].first << " hash=" << priority_list[i].second->tunnel_hash << std::endl;

        mTurtle->monitorTunnels(priority_list[i].second->tunnel_hash,this) ;

        priority_list[i].second->tunnel_status = RS_GROUTER_TUNNEL_STATUS_PENDING ;
        priority_list[i].second->last_tunnel_request_TS = now ;
    }
}

void p3GRouter::routePendingObjects()
{
    // Go throught he list of pending messages.
    // For those with a tunnel ready, send the message in the tunnel.

    time_t now = time(NULL) ;

    for(std::map<GRouterMsgPropagationId, GRouterRoutingInfo>::iterator it=_pending_messages.begin();it!=_pending_messages.end();++it)
        if(it->second.data_status == RS_GROUTER_DATA_STATUS_PENDING && it->second.tunnel_status == RS_GROUTER_TUNNEL_STATUS_READY)
        {
            const TurtleFileHash& hash(it->second.tunnel_hash) ;
            std::map<TurtleFileHash,GRouterTunnelInfo>::const_iterator vpit ;

            if( (vpit = _virtual_peers.find(hash)) != _virtual_peers.end())
            {
        // for now, just take one. But in the future, we will need some policy to temporarily store objects at proxy peers, etc.

        std::cerr << "  " << vpit->second.virtual_peers.size() << " virtual peers available. " << std::endl;

        if(vpit->second.virtual_peers.empty())
        {
            std::cerr << "  no peers available. Cannot send!!" << std::endl;
            continue ;
        }
        TurtleVirtualPeerId vpid = *(vpit->second.virtual_peers.begin()) ;

        std::cerr << "  sending to " << vpid << std::endl;

                sendDataInTunnel(vpid,it->second.data_item) ;

                it->second.data_status == RS_GROUTER_DATA_STATUS_SENT ;
                it->second.last_sent_TS = now ;
            }
        }

    // Also route back some ACKs if necessary.
    // [..]
}

void p3GRouter::sendDataInTunnel(const TurtleVirtualPeerId& vpid,RsGRouterGenericDataItem *item)
{
    NOT_IMPLEMENTED ;
}

void p3GRouter::handleIncoming()
{
    // Store and sort incoming packets, that come from the tunnels.

    RS_STACK_MUTEX(grMtx) ;

#ifdef GROUTER_DEBUG
    std::cerr << "GRouter::handleIncoming()" << std::endl;
#endif

    while(!_incoming_items.empty())
    {
        RsGRouterItem *item = _incoming_items.front() ;
        _incoming_items.pop_front() ;

        // we might receive these kinds of items: data, recept, ACK.

        NOT_IMPLEMENTED;

    delete item ;
    }
}

void p3GRouter::locked_notifyClientAcknowledged(const GRouterMsgPropagationId& msg_id,const GRouterServiceId& service_id) const
{
#ifdef GROUTER_DEBUG
	grouter_debug() << "  Key is owned by us. Notifying service that item was ACKed. msg_id=" << msg_id << ", service_id = " << service_id << "." << std::endl;
#endif
	// notify the client
	//
	std::map<GRouterServiceId,GRouterClientService*>::const_iterator its = _registered_services.find(service_id) ;

	if(its == _registered_services.end())
	{
		std::cerr << "(EE) message " << msg_id << " is attached to service " << service_id << ", which is unknown!! That is a bug." << std::endl;
		return ;
	}
	its->second->acknowledgeDataReceived(msg_id) ;
} 

void p3GRouter::addRoutingClue(const GRouterKeyId& id,const RsPeerId& peer_id)
{
    RsStackMutex mtx(grMtx) ;
#ifdef GROUTER_DEBUG
    grouter_debug() << "Received new routing clue for key " << id << " from peer " << peer_id << std::endl;
#endif
    _routing_matrix.addRoutingClue(id,peer_id,RS_GROUTER_BASE_WEIGHT_GXS_PACKET) ;
}

void p3GRouter::handleRecvDataItem(RsGRouterGenericDataItem *item)
{
	RsStackMutex mtx(grMtx) ;
    NOT_IMPLEMENTED;
}

bool p3GRouter::registerClientService(const GRouterServiceId& id,GRouterClientService *service)
{
	RsStackMutex mtx(grMtx) ;
	_registered_services[id] = service ;
	return true ;
}

bool p3GRouter::encryptDataItem(RsGRouterGenericDataItem *item,const RsGxsId& destination_key)
{
    assert(!(item->flags & RS_GROUTER_DATA_FLAGS_ENCRYPTED)) ;

#ifdef GROUTER_DEBUG
    std::cerr << "  Encrypting data for key " << destination_key << std::endl;
    std::cerr << "    Decrypted size = " << item->data_size << std::endl;
#endif
    RsTlvSecurityKey encryption_key ;

    // get the key, and let the cache find it.
    for(int i=0;i<4;++i)
        if(mIdService->getKey(destination_key,encryption_key))
            break ;
        else
            usleep(500*1000) ; // sleep half a sec.

    if(encryption_key.keyId.isNull())
    {
        std::cerr << "    (EE) Cannot get encryption key for id " << destination_key << std::endl;
        return false ;
    }

    uint8_t *encrypted_data =NULL;
    int encrypted_size =0;

    if(!GxsSecurity::encrypt(encrypted_data,encrypted_size,item->data_bytes,item->data_size,encryption_key))
    {
    std::cerr << "    (EE) Encryption failed." << std::endl;
    return false ;
    }

    delete[] item->data_bytes ;
    item->data_bytes = encrypted_data ;
    item->data_size = encrypted_size ;
    item->flags |= RS_GROUTER_DATA_FLAGS_ENCRYPTED ;

#ifdef GROUTER_DEBUG
    std::cerr << "  Encrypted size = " << encrypted_size << std::endl;
    std::cerr << "  First bytes of encrypted data: " << RsUtil::BinToHex((const char *)encrypted_data,std::min(encrypted_size,50)) << "..."<< std::endl;
    std::cerr << "  Encrypted data hash = " << RsDirUtil::sha1sum((const uint8_t *)encrypted_data,encrypted_size) << std::endl;
#endif
return true ;
}
bool p3GRouter::decryptDataItem(RsGRouterGenericDataItem *item)
{
    NOT_IMPLEMENTED ;
    return false ;
}


bool p3GRouter::signDataItem(RsGRouterGenericDataItem *item,const RsGxsId& signing_id)
{
    uint8_t *data = NULL;

    try
    {
        RsTlvSecurityKey signature_key ;

#ifdef GROUTER_DEBUG
        std::cerr << "p3GRouter::signDataItem()" << std::endl;
        std::cerr << "     Key ID = " << signing_id << std::endl;
        std::cerr << "     Getting key material..." << std::endl;
#endif
        uint32_t data_size = item->signed_data_size() ;
        uint8_t *data = (uint8_t*)malloc(data_size) ;

        if(!item->serialise_signed_data(data,data_size))
            throw std::runtime_error("Cannot serialise signed data.") ;

        if(data == NULL)
            throw std::runtime_error("Cannot allocate memory for signing data.") ;

        if(!mIdService->getPrivateKey(signing_id,signature_key))
            throw std::runtime_error("Cannot get signature key for id " + signing_id.toStdString()) ;

#ifdef GROUTER_DEBUG
        std::cerr << "     Signing..." << std::endl;
#endif
    std::cerr << "First bytes of signed data: " << RsUtil::BinToHex((const char *)data,std::min(data_size,50u)) << "..."<< std::endl;

        if(!GxsSecurity::getSignature((char *)data,data_size,signature_key,item->signature))
            throw std::runtime_error("Cannot sign for id " + signing_id.toStdString() + ". Signature call failed.") ;

    std::cerr << "Created    signature for data hash: " << RsDirUtil::sha1sum(data,data_size) << " and key id=" << signing_id << std::endl;
        free(data) ;
        return true ;
    }
    catch(std::exception& e)
    {
        std::cerr << "  signing failed. Error: " << e.what() << std::endl;
        if(data != NULL)
            free(data) ;
        item->signature.TlvClear() ;
        return false ;
    }
}
bool p3GRouter::verifySignedDataItem(RsGRouterGenericDataItem *item)
{
    uint8_t *data = NULL;

    try
    {
        RsTlvSecurityKey signature_key ;

        uint32_t data_size = item->signed_data_size() ;
        uint8_t *data = (uint8_t*)malloc(data_size) ;

        if(!item->serialise_signed_data(data,data_size))
            throw std::runtime_error("Cannot serialise signed data.") ;

        for(int i=0;i<6;++i)
            if(!mIdService->getKey(item->signature.keyId,signature_key) || signature_key.keyData.bin_data == NULL)
            {
                std::cerr << "  Cannot get key. Waiting for caching. try " << i << "/6" << std::endl;
                usleep(500 * 1000) ;	// sleep for 500 msec.
            }
            else
                break ;

        if(signature_key.keyData.bin_data == NULL)
            throw std::runtime_error("No key for checking signature from " + item->signature.keyId.toStdString());

    std::cerr << "Validating signature for data hash: " << RsDirUtil::sha1sum(data,data_size) << " and key_id = " << item->signature.keyId << std::endl;
    std::cerr << "First bytes of encrypted data: " << RsUtil::BinToHex((const char *)data,std::min(data_size,50u)) << "..."<< std::endl;

        if(!GxsSecurity::validateSignature((char*)data,data_size,signature_key,item->signature))
            throw std::runtime_error("Signature was verified and it doesn't check! This is a security issue!") ;

    free(data) ;
        return true ;
    }
    catch(std::exception& e)
    {
        std::cerr << "  signature verification failed. Error: " << e.what() << std::endl;
        if(data != NULL)
            free(data) ;
        return false ;
    }
}

bool p3GRouter::sendData(const RsGxsId& destination,const GRouterServiceId& client_id,uint8_t *data, uint32_t data_size,const RsGxsId& signing_id, GRouterMsgPropagationId &propagation_id)
{
    // Make sure we have a unique id (at least locally).
    //
    {
        RsStackMutex mtx(grMtx) ;
        do { propagation_id = RSRandom::random_u64(); } while(_pending_messages.find(propagation_id) != _pending_messages.end()) ;
    }

    // create the signed data item

    RsGRouterGenericDataItem *data_item = new RsGRouterGenericDataItem ;

    data_item->data_bytes = data ;
    data_item->data_size = data_size ;
    data_item->routing_id = propagation_id  ;
    data_item->randomized_distance = 0 ;
    data_item->destination_key = destination  ;
    data_item->flags = 0  ;

    // First, encrypt.

    if(!encryptDataItem(data_item,destination))
    {
        std::cerr << "Cannot encrypt data item. Some error occured!" << std::endl;
        return false;
    }

    // Then, sign the encrypted data, so that the signature can be checked by non priviledged users.

    if(!signDataItem(data_item,signing_id))
    {
        std::cerr << "Cannot sign data item. Some error occured!" << std::endl;
        return false;
    }

    // Verify the signature. If that fails, there's a bug somewhere!!

    if(!verifySignedDataItem(data_item))
    {
        std::cerr << "Cannot verify data item that was just signed. Some error occured!" << std::endl;
        return false;
    }
    // push the item into pending messages.
    //
    GRouterRoutingInfo info ;

    time_t now = time(NULL) ;

    info.data_item = data_item ;
    info.data_status = RS_GROUTER_DATA_STATUS_PENDING ;
    info.tunnel_status = RS_GROUTER_TUNNEL_STATUS_UNMANAGED ;
    info.last_sent_TS = 0 ;
    info.last_tunnel_request_TS = 0 ;
    info.sending_attempts = 0 ;
    info.received_time_TS = now ;
    info.destination_key = destination ;
    info.tunnel_hash = makeTunnelHash(destination,client_id) ;
    info.client_id = client_id ;

#ifdef GROUTER_DEBUG
    grouter_debug() << "p3GRouter::sendGRouterData(): pushing the followign item in the msg pending list:" << std::endl;
    grouter_debug() << "  routing id     = " << propagation_id << std::endl;
    grouter_debug() << "  data_item.size = " << info.data_item->data_size << std::endl;
    grouter_debug() << "  data_item.byte = " << RsDirUtil::sha1sum(info.data_item->data_bytes,info.data_item->data_size) << std::endl;
    grouter_debug() << "  destination    = " << info.destination_key << std::endl;
    grouter_debug() << "  signed by key  = " << data_item->signature.keyId << std::endl;
    grouter_debug() << "  data status    = " << info.data_status << std::endl;
    grouter_debug() << "  tunnel status  = " << info.tunnel_status << std::endl;
    grouter_debug() << "  sending attempt= " << info.sending_attempts << std::endl;
    grouter_debug() << "  distance       = " << info.data_item->randomized_distance << std::endl;
    grouter_debug() << "  recv time      = " << info.received_time_TS << std::endl;
    grouter_debug() << "  client id      = " << std::hex << info.client_id << std::dec << std::endl;
    grouter_debug() << "  tunnel hash    = " << info.tunnel_hash << std::endl;
#endif

    {
        RsStackMutex mtx(grMtx) ;
        _pending_messages[propagation_id] = info ;
    }
return true ;
}

Sha1CheckSum p3GRouter::makeTunnelHash(const RsGxsId& destination,const GRouterServiceId& client)
{
    assert(  destination.SIZE_IN_BYTES == 16) ;
    assert(Sha1CheckSum::SIZE_IN_BYTES == 20) ;

    uint8_t bytes[20] ;
    memcpy(bytes,destination.toByteArray(),16) ;
    bytes[16] = 0 ;
    bytes[17] = 0 ;
    bytes[18] = (client >> 8) & 0xff ;
    bytes[19] =  client       & 0xff ;

    return Sha1CheckSum(bytes) ;
}

bool p3GRouter::loadList(std::list<RsItem*>& items)
{
	RsStackMutex mtx(grMtx) ;

#ifdef GROUTER_DEBUG
	grouter_debug() << "p3GRouter::loadList() : " << std::endl;
#endif

	_routing_matrix.loadList(items) ;

#ifdef GROUTER_DEBUG
	// remove all existing objects.
	//
	grouter_debug() << "  removing all existing items (" << _pending_messages.size() << " items to delete)." << std::endl;
#endif

#ifdef SUSPENDED
	// clear the existing list.
	//
	for(std::map<GRouterMsgPropagationId,GRouterRoutingInfo>::iterator it(_pending_messages.begin());it!=_pending_messages.end();++it)
        delete it->second.data_item ;

	_pending_messages.clear() ;

	for(std::list<RsItem*>::const_iterator it(items.begin());it!=items.end();++it)
	{
		RsGRouterRoutingInfoItem *itm1 = NULL ;

		if(NULL != (itm1 = dynamic_cast<RsGRouterRoutingInfoItem*>(*it)))
		{
			_pending_messages[itm1->data_item->routing_id] = *itm1 ;	
			//_pending_messages[itm1->data_item->routing_id].data_item = itm1->data_item ;	// avoids duplication.

			itm1->data_item = NULL ;	// prevents deletion.
		}

		delete *it ;
    }
#endif
	return true ;
}
bool p3GRouter::saveList(bool& cleanup,std::list<RsItem*>& items) 
{
	// We save
	// 	- the routing clues
	// 	- the pending items

	cleanup = true ; // the client should delete the items.

#ifdef GROUTER_DEBUG
	grouter_debug() << "p3GRouter::saveList()..." << std::endl;
	grouter_debug() << "  saving routing clues." << std::endl;
#endif

	_routing_matrix.saveList(items) ;

#ifdef GROUTER_DEBUG
    grouter_debug() << "  saving pending items." << std::endl;
#endif

#ifdef SUSPENDED
	for(std::map<GRouterMsgPropagationId,GRouterRoutingInfo>::const_iterator it(_pending_messages.begin());it!=_pending_messages.end();++it)
	{
		RsGRouterRoutingInfoItem *item = new RsGRouterRoutingInfoItem ;
		
		*(GRouterRoutingInfo*)item = it->second ;	// copy all members

		item->data_item = it->second.data_item->duplicate() ;	// deep copy, because we call delete on the object, and the item might be removed before we handle it in the client.
		items.push_back(item) ;
    }
#endif

	return true ;
}

bool p3GRouter::getRoutingMatrixInfo(RsGRouter::GRouterRoutingMatrixInfo& info) 
{
	info.per_friend_probabilities.clear() ;
	info.friend_ids.clear() ;
	info.published_keys.clear() ;

	std::set<RsPeerId> ids ;
	mServiceControl->getPeersConnected(getServiceInfo().mServiceType,ids) ;

	RsStackMutex mtx(grMtx) ;

    //info.published_keys = _owned_key_ids ;

	for(std::set<RsPeerId>::const_iterator it(ids.begin());it!=ids.end();++it)
		info.friend_ids.push_back(*it) ;

	std::vector<GRouterKeyId> known_keys ;
	std::vector<float> probas ;
	_routing_matrix.getListOfKnownKeys(known_keys) ;

	for(uint32_t i=0;i<known_keys.size();++i)
	{
		_routing_matrix.computeRoutingProbabilities(known_keys[i],info.friend_ids,probas) ;
		info.per_friend_probabilities[known_keys[i]] = probas ;
	}

	return true ;
}
bool p3GRouter::getRoutingCacheInfo(std::vector<GRouterRoutingCacheInfo>& infos)  
{
	RsStackMutex mtx(grMtx) ;
	infos.clear() ;

	for(std::map<GRouterMsgPropagationId,GRouterRoutingInfo>::const_iterator it(_pending_messages.begin());it!=_pending_messages.end();++it)
	{
		infos.push_back(GRouterRoutingCacheInfo()) ;
		GRouterRoutingCacheInfo& cinfo(infos.back()) ;

		cinfo.mid = it->first ;
		cinfo.destination = it->second.destination_key ;
        cinfo.time_stamp = it->second.received_time_TS ;
        cinfo.status = it->second.data_status;
		cinfo.data_size = it->second.data_item->data_size ;
	}
	return true ;
}

// Dump everything
//
void p3GRouter::debugDump()
{
	RsStackMutex mtx(grMtx) ;

	time_t now = time(NULL) ;

	grouter_debug() << "Full dump of Global Router state: " << std::endl; 
	grouter_debug() << "  Owned keys : " << std::endl;

    for(std::map<Sha1CheckSum, GRouterPublishedKeyInfo>::const_iterator it(_owned_key_ids.begin());it!=_owned_key_ids.end();++it)
	{
        grouter_debug() << "    Hash            : " << it->first << std::endl;
		grouter_debug() << "      Service id    : " << std::hex << it->second.service_id << std::dec << std::endl;
		grouter_debug() << "      Description   : " << it->second.description_string << std::endl;
	}

	grouter_debug() << "  Registered services: " << std::endl;

	for(std::map<GRouterServiceId,GRouterClientService *>::const_iterator it(_registered_services.begin() );it!=_registered_services.end();++it)
		grouter_debug() << "    " << std::hex << it->first << "   " << std::dec << (void*)it->second << std::endl;

	grouter_debug() << "  Data items: " << std::endl;

	static const std::string statusString[5] = { "Unkn","Pend","Sent","Ackn","Dead" };

	for(std::map<GRouterMsgPropagationId, GRouterRoutingInfo>::iterator it(_pending_messages.begin());it!=_pending_messages.end();++it)
    {
        grouter_debug() << "    Msg id     : " << std::hex << it->first << std::dec ;
        grouter_debug() << "    Destination: " << it->second.destination_key ;
        grouter_debug() << "    Received   : " << now - it->second.received_time_TS << " secs ago.";
        grouter_debug() << "    Last sent  : " << now - it->second.last_sent_TS << " secs ago.";
        grouter_debug() << "    Data Status: " << statusString[it->second.data_status] << std::endl;
        grouter_debug() << "    Tunl Status: " << statusString[it->second.tunnel_status] << std::endl;
    }

    grouter_debug() << "  Tunnels: " << std::endl;

    for(std::map<TurtleFileHash,GRouterTunnelInfo>::const_iterator it(_virtual_peers.begin());it!=_virtual_peers.end();++it)
    {
        grouter_debug() << "    hash: " << it->first << ", first received: " << now - it->second.last_tunnel_ok_TS << " (secs ago), last received: " << now - it->second.last_tunnel_ok_TS << std::endl;

        for(std::set<TurtleVirtualPeerId>::const_iterator it2 = it->second.virtual_peers.begin();it2!=it->second.virtual_peers.end();++it2)
            grouter_debug() << "      " << *it2 << std::endl;
    }

	grouter_debug() << "  Routing matrix: " << std::endl;

    if(_debug_enabled)
        _routing_matrix.debugDump() ;
}




