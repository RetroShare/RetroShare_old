#pragma once

#include <string>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <stdint.h>
#include <turtle/p3turtle.h>

class MonitoredTurtleRouter ;
class MonitoredTurtleClient ;
class RsTurtle ;
class RsRawItem ;
class p3ServiceServer ;

class PeerNode
{
	public:
		struct NodeTrafficInfo
		{
			std::map<std::string,std::string> local_src ;	// peer id., tunnel id
			std::map<std::string,std::string> local_dst ;
		};

		PeerNode(const RsPeerId& id,const std::list<RsPeerId>& friends) ;
		~PeerNode() ;

		RsRawItem *outgoing() ;
		void incoming(RsRawItem *) ;

		const RsPeerId& id() const { return _id ;}

		void tick() ;

		const RsTurtle *turtle_service() const ;

		// Turtle-related methods
		//
		void manageFileHash(const RsFileHash& hash) ;
		void provideFileHash(const RsFileHash& hash) ;

		const std::set<TurtleFileHash>& providedHashes() const { return _provided_hashes; }
		const std::set<TurtleFileHash>& managedHashes() const { return _managed_hashes; }

		void getTrafficInfo(NodeTrafficInfo& trinfo) ;	// 

	private:
		p3ServiceServer *_service_server ;
        p3turtle *_turtle ;
        MonitoredTurtleClient *_ftserver ;
		pqiPublisher *_publisher ;
		RsPeerId _id ;

		std::set<TurtleFileHash> _provided_hashes ;
		std::set<TurtleFileHash> _managed_hashes ;
};

template<class NODE_TYPE> class Graph
{
	public:
		typedef uint32_t NodeId ;

		void symmetric_connect(uint32_t n1,uint32_t n2) ;
		const std::set<NodeId>& neighbors(const NodeId& n) const { return _neighbors[n] ; }

		// number of nodes.
		//
		uint32_t n_nodes() const { return _nodes.size() ;}
		const NODE_TYPE& node(int i) const { return *_nodes[i] ; }
		NODE_TYPE& node(int i) { return *_nodes[i] ; }

	protected:

		std::vector<NODE_TYPE *> _nodes ;
		std::vector<std::set<uint32_t> > _neighbors ;
};

class Network: public Graph<PeerNode>
{
	public:
		// inits the graph as random. Returns true if connected, false otherwise.
		//
		bool initRandom(uint32_t n_nodes, float connexion_probability) ;

		// ticks all services of all nodes.
		//
		void tick() ;

		PeerNode& node_by_id(const RsPeerId& node_id) ;

	private:
		std::map<RsPeerId,uint32_t> _node_ids ;
};

