
/*
 * libretroshare/src/serialiser: rsturtleitems_test.cc
 *
 * RetroShare Serialiser.
 *
 * Copyright 2007-2008 by Cyril Soler
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

/******************************************************************
 */

#include <iostream>
#include <sstream>
#include "turtle/rsturtleitem.h"
#include <serialiser/rstlvutil.h>
#include "support.h"
#include "util/utest.h"

INITTEST();

RsSerialType* init_item(CompressedChunkMap& map)
{
	map._map.clear() ;
	for(uint32_t i=0;i<15;++i)
		map._map.push_back(rand()) ;

	return new RsTurtleSerialiser();
}
bool operator==(const CompressedChunkMap& m1,const CompressedChunkMap& m2)
{
	if(m1._map.size() != m2._map.size()) return false ;

	for(uint32_t i=0;i<m1._map.size();++i)
		if(m1._map[i] != m2._map[i])
			return false ;

	return true ;
}
bool operator==(const RsTurtleFileMapRequestItem& it1,const RsTurtleFileMapRequestItem& it2)
{
	if(it1.direction != it2.direction) return false ;
	if(it1.tunnel_id != it2.tunnel_id) return false ;

	return true ;
}
RsSerialType* init_item(RsTurtleFileMapRequestItem& item)
{
	item.direction = 1 ;
	item.tunnel_id = 0x4ff823e2 ;
	return new RsTurtleSerialiser();
}
bool operator==(const RsTurtleFileMapItem& it1,const RsTurtleFileMapItem& it2)
{
	if(it1.direction != it2.direction) return false ;
	if(it1.tunnel_id != it2.tunnel_id) return false ;
	if(!(it1.compressed_map == it2.compressed_map)) return false ;

	return true ;
}
RsSerialType* init_item(RsTurtleFileMapItem& item)
{
	item.direction = 1 ;
	item.tunnel_id = 0xf48fe232 ;
	init_item(item.compressed_map) ;
	return new RsTurtleSerialiser();
}
RsSerialType* init_item(RsTurtleFileDataItem& item)
{
	static const uint32_t S = 3456 ;
	item.tunnel_id = 0x33eef982 ;
	item.chunk_offset = 0x25ea228437894379ull ;
	item.chunk_size = S ;
	item.chunk_data = new unsigned char[S] ;
	for(uint32_t i=0;i<S;++i)
		((unsigned char *)item.chunk_data)[i] = rand()%256 ;
	return new RsTurtleSerialiser();
}
bool operator==(const RsTurtleFileDataItem& i1,const RsTurtleFileDataItem& i2)
{
	if(i1.tunnel_id != i2.tunnel_id) return false ;
	if(i1.chunk_offset != i2.chunk_offset) return false ;
	if(i1.chunk_size != i2.chunk_size) return false ;
	for(uint32_t i=0;i<i1.chunk_size;++i)
		if( ((unsigned char *)i1.chunk_data)[i] != ((unsigned char *)i2.chunk_data)[i])
			return false ;
	return true ;
}
RsSerialType* init_item(RsTurtleFileRequestItem& item)
{
	item.tunnel_id = rand() ;
	item.chunk_offset = 0x25ea228437894379ull ;
	item.chunk_size = rand() ;
	return new RsTurtleSerialiser();
}
bool operator==(const RsTurtleFileRequestItem& it1,const RsTurtleFileRequestItem& it2)
{
	if(it1.tunnel_id != it2.tunnel_id) return false ;
	if(it1.chunk_offset != it2.chunk_offset) return false ;
	if(it1.chunk_size != it2.chunk_size) return false ;

	return true ;
}
RsSerialType* init_item(RsTurtleTunnelOkItem& item)
{
	item.tunnel_id = rand() ;
	item.request_id = rand() ;
	return new RsTurtleSerialiser();
}
bool operator==(const RsTurtleTunnelOkItem& it1,const RsTurtleTunnelOkItem& it2)
{
	if(it1.tunnel_id != it2.tunnel_id) return false ;
	if(it1.request_id != it2.request_id) return false ;
	return true ;
}
RsSerialType* init_item(RsTurtleOpenTunnelItem& item)
{
	item.depth = rand() ;
	item.request_id = rand() ;
	item.partial_tunnel_id = rand() ;
	item.file_hash = std::string("c0edcfecc0844ef175d61dd589ab288d262b6bc8") ;
	return new RsTurtleSerialiser();
}
bool operator==(const RsTurtleOpenTunnelItem& it1,const RsTurtleOpenTunnelItem& it2)
{
	if(it1.depth != it2.depth) return false ;
	if(it1.request_id != it2.request_id) return false ;
	if(it1.partial_tunnel_id != it2.partial_tunnel_id) return false ;
	if(it1.file_hash != it2.file_hash) return false ;
	return true ;
}
RsSerialType* init_item(RsTurtleRegExpSearchRequestItem& item)
{
	item.request_id = rand() ;
	item.depth = rand() ;
	item.expr._tokens.clear() ;
	item.expr._ints.clear() ;
	item.expr._strings.clear() ;

	for(uint32_t i=0;i<10u;++i) item.expr._tokens.push_back(rand()%8) ;
	for(uint32_t i=0;i<6u;++i) item.expr._ints.push_back(rand()) ;
	for(uint32_t i=0;i<8u;++i) item.expr._strings.push_back("test string") ;
	return new RsTurtleSerialiser();
}
bool operator==(const RsTurtleRegExpSearchRequestItem& it1,const RsTurtleRegExpSearchRequestItem& it2)
{
	if(it1.request_id != it2.request_id) return false ;
	if(it1.depth != it2.depth) return false ;
	if(it1.expr._tokens.size() != it2.expr._tokens.size()) return false ;
	if(it1.expr._ints.size() != it2.expr._ints.size()) return false ;
	if(it1.expr._strings.size() != it2.expr._strings.size()) return false ;
	for(uint32_t i=0;i<it1.expr._tokens.size();++i) if(it1.expr._tokens[i] != it2.expr._tokens[i]) return false ;
	for(uint32_t i=0;i<it1.expr._ints.size();++i) if(it1.expr._ints[i] != it2.expr._ints[i]) return false ;
	for(uint32_t i=0;i<it1.expr._strings.size();++i) if(it1.expr._strings[i] != it2.expr._strings[i]) return false ;
	return true ;
}
RsSerialType* init_item(RsTurtleStringSearchRequestItem& item)
{
	item.request_id = rand() ;
	item.depth = rand() ;
	item.match_string = std::string("432hkjfdsjkhjk43r3fw") ;
	return new RsTurtleSerialiser();
}
bool operator==(const RsTurtleStringSearchRequestItem& it1,const RsTurtleStringSearchRequestItem& it2)
{
	if(it1.request_id != it2.request_id) return false ;
	if(it1.depth != it2.depth) return false ;
	if(it1.match_string != it2.match_string)
		return false ;
	return true ;
}
RsSerialType* init_item(TurtleFileInfo& info)
{
	info.hash = "3f753e8ac3b94ab9fddfad94480f747bf4418370";
	info.name = "toto.png";
	info.size = 0x3392085443897ull ;
	return new RsTurtleSerialiser();
}
bool operator==(const TurtleFileInfo& it1,const TurtleFileInfo& it2)
{
	if(it1.hash != it2.hash) return false ;
	if(it1.name != it2.name) return false ;
	if(it1.size != it2.size) return false ;
	return true ;
}
RsSerialType* init_item(RsTurtleSearchResultItem& item)
{
	item.depth = rand() ;
	item.request_id = rand() ;
	item.result.clear() ;
	static const uint32_t S = 10 ;
	for(uint32_t i=0;i<S;++i)
	{
		TurtleFileInfo f;
		init_item(f) ;
		item.result.push_back(f) ;
	}
	return new RsTurtleSerialiser();
}
bool operator==(const RsTurtleSearchResultItem& it1,const RsTurtleSearchResultItem& it2)
{
	if(it1.request_id != it2.request_id) return false ;
	if(it1.depth != it2.depth) return false ;

	std::list<TurtleFileInfo>::const_iterator i1(it1.result.begin()) ;
	std::list<TurtleFileInfo>::const_iterator i2(it2.result.begin()) ;

	for(;i1!=it1.result.end() && i2!=it2.result.end();++i1,++i2)
		if( !(*i1 == *i2))
			return false ;

	return true ;
}

int main()
{
	std::cerr << "RsTurtleItem Tests" << std::endl;

	for(uint32_t i=0;i<20;++i)
	{
		test_RsItem<RsTurtleFileMapRequestItem>(); REPORT("Serialise/Deserialise RsTurtleFileMapRequestItem");
		test_RsItem<RsTurtleFileMapItem       >(); REPORT("Serialise/Deserialise RsTurtleFileMapItem");
		test_RsItem<RsTurtleFileDataItem      >(); REPORT("Serialise/Deserialise RsTurtleFileDataItem");
		test_RsItem<RsTurtleFileRequestItem   >(); REPORT("Serialise/Deserialise RsTurtleFileRequestItem");
		test_RsItem<RsTurtleTunnelOkItem      >(); REPORT("Serialise/Deserialise RsTurtleTunnelOkItem   ");
		test_RsItem<RsTurtleOpenTunnelItem    >(); REPORT("Serialise/Deserialise RsTurtleOpenTunnelItem ");
		test_RsItem<RsTurtleSearchResultItem  >(); REPORT("Serialise/Deserialise RsTurtleSearchResultItem ");
		test_RsItem<RsTurtleStringSearchRequestItem    >(); REPORT("Serialise/Deserialise RsTurtleStringSearchRequestItem ");
		test_RsItem<RsTurtleRegExpSearchRequestItem    >(); REPORT("Serialise/Deserialise RsTurtleRegExpSearchRequestItem ");
	}
	
	FINALREPORT("RsturtleItem Tests");

	return TESTRESULT();
}


