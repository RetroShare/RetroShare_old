/*
 * libretroshare/src/ft/ftfilemapper.h
 *
 * File Transfer for RetroShare.
 *
 * Copyright 2011 by Cyril Soler.  
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

#pragma once

// This class implements data storage for incoming files. It provides the following functionality:
//
// - linear storage of data
// - record of which order the data is currently stored into
// - automatic (and clever) re-organisation of data as it arrives
//
// The implementation ensures that:
// - when the file is complete, the data is always ordered correctly
// - when data comes, the writes always happen within the range of existing data, plus at most one chunk.
//
// Using this class avoids writting in the middle of large files while downloading them, which removes the lag
// of RS interface when starting the DL of a large file.
//
// The re-organisation of the file data occurs seamlessly during writes, and therefore does not cause any big 
// freeze at end of download.
//
// As soon as the first chunk has been downloaded, it is possible to preview a file by directly playing 
// the partial file, so no change is needed to preview.
//
#include <stdint.h>
#include <string>
#include <vector>

class CompressedChunkMap ;

class ftFileMapper
{
	public:
		// Name and size of the file to store. This will usually be a file in the Partials/ directory.
		ftFileMapper(uint64_t file_size,uint32_t chunk_size) ;

		// Storage/retreive of data. All offsets are given in absolute position in the file. The class handles
		// the real mapping (hence the name).
		
		// initialization from saved data.

		void initMappedChunks(uint64_t file_size,const CompressedChunkMap& cmap,const std::vector<uint32_t>& data_chunks_ids) ;

		// Stores the data in the file, at the given offset. The chunk does not necessarily exist. If not, 
		// the data is written at the end of the current file. If yes, it is written at the actual place.
		// Returned values:
		//
		//		true: 	the data has correctly been written
		//		false: 	the data could not be written
		//
		bool storeData(void *data, uint32_t data_size, uint64_t offset,FILE *fd) ;

		// Gets the data from the storage file. The data should be there. The data is stored into buff, which needs to be
		// allocated by the client. Returned values:
		//
		//		true: 	the data has correctly been read
		//		false: 	the data could not beread, or does not exist.
		//
		bool retrieveData(void *data, uint32_t data_size, uint64_t offset,FILE *fd) ;

		// Returns the number of each chunk that is mapped to place 0,1,2,... on the disk.
		//
		const std::vector<uint32_t>& getMappedChunks() const { return _data_chunk_ids ; }

		// debug
		void print() const ;
		bool consistencyTest() const ;

		virtual void forceCheckPartialFile() {}
	private:
		uint64_t _file_size ;	// size of the file
		uint32_t _chunk_size ;	// size of chunks

		// List of chunk ids (0,1,2,3...) stored in the order. Two arrays represent the same info for efficiency reasons. We have
		//
		// 	_mapped_chunks[_data_chunks[i]] = i
		// 	_data_chunks[_mapped_chunks[i]] = i
		//
		std::vector<int> _mapped_chunks ;	 //  Index into the Partial file of where each chunk is stored. -1 for unmapped chunk. Has 
		                                     // size equal to number of chunks.
		std::vector<uint32_t> _data_chunk_ids ; //  index of the chunk that ends up at each place of the partials file. Has size n were n 
														 	 // is the number of downloaded chunks.  _first_free_chunk = _data_chunks.size().

		bool writeData(uint64_t offset,uint32_t size,void *data,FILE *fd) const ;
		bool readData(uint64_t offset,uint32_t size,void *data,FILE *fd) const ;
		bool wipeChunk(uint32_t cid,FILE *fd) const ;

		bool computeStorageOffset(uint64_t offset,uint64_t& storage_offset) const ;

		bool moveChunk(uint32_t src,uint32_t dst,FILE *fd_out) ;
		uint32_t allocateNewEmptyChunk(FILE *fd) ;
};

