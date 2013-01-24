#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include "ftfilemapper.h"
#include "ftchunkmap.h"

#define DEBUG_FILEMAPPER

ftFileMapper::ftFileMapper(uint64_t file_size,uint32_t chunk_size) 
	: _file_size(file_size),_chunk_size(chunk_size)
{
	int nb_chunks = (int)(file_size / (uint64_t)chunk_size) + ( (file_size % chunk_size)==0 ?0:1 ) ;

#ifdef DEBUG_FILEMAPPER
	std::cerr << "(DD) Creating ftFileMapper for file of size " << file_size << ", with " << nb_chunks << " chunks." << std::endl;
#endif

	_mapped_chunks.clear() ;
	_mapped_chunks.resize(nb_chunks,-1) ;
	_data_chunk_ids.clear() ;
	
#ifdef DEBUG_FILEMAPPER
	consistencyTest();
#endif
}

bool ftFileMapper::computeStorageOffset(uint64_t offset,uint64_t& storage_offset) const
{
	// Compute the chunk number for this offset
	//
	uint32_t cid = (uint32_t)(offset / (uint64_t)_chunk_size) ;

	// Check that the cid is in the allowed range. That should always be the case.
	//
	if(cid < _mapped_chunks.size() && _mapped_chunks[cid] >= 0)
	{
		storage_offset = _mapped_chunks[cid]*_chunk_size + (offset % (uint64_t)_chunk_size) ;
		return true ;
	}
	else
	{
#ifdef DEBUG_FILEMAPPER
		std::cerr << "(DD) ftFileMapper::computeStorageOffset(): offset " << offset << " corresponds to chunk number " << cid << " which is not mapped!!" << std::endl;
#endif
		return false ;
	}
}

bool ftFileMapper::readData(uint64_t offset,uint32_t size,void *data,FILE *fd) const
{
	if (0 != fseeko64(fd, offset, SEEK_SET))
	{
		std::cerr << "(EE) ftFileMapper::ftFileMapper::readData() Bad fseek at offset " << offset << ", fd=" << (void*)fd << ", size=" << size << ", errno=" << errno << std::endl;
		return false;
	}

	if (1 != fread(data, size, 1, fd))
	{
		std::cerr << "(EE) ftFileMapper::readData() Bad fread." << std::endl;
		std::cerr << "ERRNO: " << errno << std::endl;

		return false;
	}
	return true ;
}

bool ftFileMapper::writeData(uint64_t offset,uint32_t size,void *data,FILE *fd) const
{
	if (0 != fseeko64(fd, offset, SEEK_SET))
	{
		std::cerr << "(EE) ftFileMapper::ftFileMapper::writeData() Bad fseek at offset " << offset << ", fd=" << (void*)fd << ", size=" << size << ", errno=" << errno << std::endl;
		return false;
	}

	if (1 != fwrite(data, size, 1, fd))
	{
		std::cerr << "(EE) ftFileMapper::ftFileCreator::addFileData() Bad fwrite." << std::endl;
		std::cerr << "ERRNO: " << errno << std::endl;

		return false;
	}
	fflush(fd) ;
	return true ;
}

bool ftFileMapper::storeData(void *data, uint32_t data_size, uint64_t offset,FILE *fd) 
{
	uint64_t real_offset = 0;
	
#ifdef DEBUG_FILEMAPPER
	std::cerr << "(DD) ftFileMapper::storeData(): storing data size " << data_size << " for offset "<< offset << std::endl;
#endif

	// we compute the real place of the data in the mapped file. Several cases:
	//
	// 1 - the place corresponds to a mapped place
	// 				=> write there.
	// 2 - the place does not correspond to a mapped place. 
	// 	2.0 - we allocate a new chunk at the end of the file. 
	// 			2.0.1 - the chunk corresponds to a mapped chunk somewhere before
	// 							=> we move it, and use the other chunk as writing position
	// 			2.0.2 - the chunk does not correspond to a mapped chunk somewhere before
	// 			            => we use it
	// 	2.1 - the place is in the range of existing data
	// 				=> we move the existing data at the end of the file, and update the mapping
	// 	2.2 - the place is outside the range of existing data
	// 				=> we allocate a new chunk at the end of the file, and write there.
	// 					2.2.1 - we look for the first chunk that is not already mapped before.
	//
	if(!computeStorageOffset(offset,real_offset))
	{
		uint32_t cid = (uint32_t)(offset / (uint64_t)_chunk_size) ;

#ifdef DEBUG_FILEMAPPER
		std::cerr << "(DD)   real offset unknown. chunk id is " << cid << std::endl;
#endif

		uint32_t empty_chunk = allocateNewEmptyChunk(fd) ;

#ifdef DEBUG_FILEMAPPER
		std::cerr << "(DD)   allocated new empty chunk " << empty_chunk << std::endl;
#endif

		if(cid < _data_chunk_ids.size() && cid != empty_chunk)	// the place is already occupied by some data
		{
#ifdef DEBUG_FILEMAPPER
			std::cerr << "(DD)   chunk already in use. " << std::endl;
			std::cerr << "(DD)   swapping with first free chunk: " << empty_chunk << std::endl;
#endif

			if(!moveChunk(cid, empty_chunk,fd))
			{
				std::cerr << "(EE) ftFileMapper::writeData(): cannot move chunk " << empty_chunk << " and " << cid << std::endl ;
				return false ;
			}

			// Get the old chunk id that was mapping to this place
			//
			int oid = _data_chunk_ids[cid] ;

			if(oid < 0)
			{
				std::cerr << "(EE) ftFileMapper::writeData(): cannot find chunk that was previously mapped to place " << cid << std::endl ;
				return false ;
			}
#ifdef DEBUG_FILEMAPPER
			std::cerr << "(DD)   old chunk now pointing to: " << empty_chunk << std::endl;
			std::cerr << "(DD)   new chunk now pointing to: " << cid << std::endl;
#endif

			_mapped_chunks[cid] = cid ; // this one is in place, since we swapped it
			_mapped_chunks[oid] = empty_chunk ;
			_data_chunk_ids[cid] = cid ;
			_data_chunk_ids[empty_chunk] = oid ;
		}
		else // allocate a new chunk at end of the file.
		{
#ifdef DEBUG_FILEMAPPER
			std::cerr << "(DD)   allocating new storage place at first free chunk: " << empty_chunk << std::endl;
#endif

			_mapped_chunks[cid] = empty_chunk ;
			_data_chunk_ids[empty_chunk] = cid ;
		}

#ifdef DEBUG_FILEMAPPER
		consistencyTest();
#endif

		real_offset = _mapped_chunks[cid]*(uint64_t)_chunk_size + (offset % (uint64_t)_chunk_size) ;
	}
#ifdef DEBUG_FILEMAPPER
	std::cerr << "(DD)   real offset = " << real_offset << ", data size=" << data_size << std::endl;
	std::cerr << "(DD)   writing data " << std::endl;
#endif

	return writeData(real_offset,data_size,data,fd) ;
}

bool ftFileMapper::retrieveData(void *data, uint32_t data_size, uint64_t offset,FILE *fd)
{
	uint64_t storage_offset ;

	if(!computeStorageOffset(offset,storage_offset) )
	{
		std::cerr << "(EE) ftFileMapper::retrieveData(): attempt to get unmapped data at offset " << offset << std::endl;
		return false ;
	}
	return readData(storage_offset,data_size,data,fd) ;
}

uint32_t ftFileMapper::allocateNewEmptyChunk(FILE *fd_out)
{
	// look into first_free_chunk. Is it the place of a chunk already mapped before?
	//
#ifdef DEBUG_FILEMAPPER
	std::cerr << "(DD) ftFileMapper::getFirstEmptyChunk()" << std::endl;
#endif

	uint32_t first_free_chunk = _data_chunk_ids.size() ;
	_data_chunk_ids.push_back(first_free_chunk) ;

	if(_mapped_chunks[first_free_chunk] >= 0 && _mapped_chunks[first_free_chunk] < (int)first_free_chunk)
	{
		uint32_t old_chunk = _mapped_chunks[first_free_chunk] ;

#ifdef DEBUG_FILEMAPPER
		std::cerr << "(DD)   first free chunk " << first_free_chunk << " is actually mapped to " <<  old_chunk << ". Moving it." << std::endl;
#endif

		moveChunk(old_chunk,first_free_chunk,fd_out) ;
		_mapped_chunks[first_free_chunk] = first_free_chunk ;

		// After that, the consistency is broken, because 
		// 	_data_chunk_ids[old_chunk] = _data_chunk_ids[first_free_chunk] = first_free_chunk
		// However, we cannot give a sensible value to _data_chunk_ids[old_chunk] because it's going to 
		// be attributed by the client.

#ifdef DEBUG_FILEMAPPER
		std::cerr << "(DD)   Returning " << old_chunk << std::endl;
#endif

		return old_chunk ;
	}
	else
	{
#ifdef DEBUG_FILEMAPPER
		std::cerr << "(DD)   first free chunk is fine. Returning " << first_free_chunk << ", and making room" << std::endl;
#endif
		
		// We need to wipe the entire chunk, since it might be moved before beign completely written, which would cause
		// a fread error.
		//
		wipeChunk(first_free_chunk,fd_out) ;

		return first_free_chunk ;
	}
}

bool ftFileMapper::wipeChunk(uint32_t cid,FILE *fd) const
{
	uint32_t size = (cid == _mapped_chunks.size()-1)?(_file_size - cid*_chunk_size) : _chunk_size ;

	void *buf = malloc(size) ;
	memset(buf,0,size) ;		// Avoids uninitialized memory read below.

	if(buf == NULL)
	{
		std::cerr << "(EE) ftFileMapper::wipeChunk(): cannot allocate temporary buf of size " << size << std::endl;
		return false ;
	}

	if(fseeko64(fd, cid*_chunk_size, SEEK_SET)!= 0)
	{
		std::cerr << "(EE) ftFileMapper::wipeChunk(): cannot fseek file at position " << cid*_chunk_size << std::endl;
		free(buf) ;
		return false ;
	}

	if(1 != fwrite(buf, size, 1, fd))
	{
		std::cerr << "(EE) ftFileMapper::wipeChunk(): cannot write to file" << std::endl;
		free(buf) ;
		return false ;
	}

	free(buf) ;
	return true ;
}

void ftFileMapper::initMappedChunks(uint64_t file_size,const CompressedChunkMap& cmap,const std::vector<uint32_t>& data_chunk_ids)
{
	uint32_t numck = ChunkMap::getNumberOfChunks(file_size) ;
	uint32_t count = cmap.countFilledChunks(numck) ;

	_mapped_chunks.clear() ;
	_mapped_chunks.resize(numck,-1) ;
	_data_chunk_ids.clear() ;

	// 0 - retro-compatibility. First check that the number of chunks in both maps co-incide.

	if(data_chunk_ids.size() < count)
	{
		std::cerr << "(II) ftFileMapper::initMappedChunks(): file has " << count << " chunks on disk, but no mapping. Assuming it's an unfragmented file (backward compatibility)!" << std::endl;
		_data_chunk_ids.resize(numck) ;

		for(uint32_t i=0;i<numck;++i)
		{
			_data_chunk_ids[i] = i ;
			_mapped_chunks[i] = i ;
		}
	}
	else 
	{
		std::cerr << "(II) ftFileMapper::initMappedChunks(): file has " << count << " chunks on disk. Data ids is fine." << std::endl;

		_data_chunk_ids = data_chunk_ids;

		for(uint32_t i=0;i<data_chunk_ids.size();++i)
			_mapped_chunks[data_chunk_ids[i]] = i ;
	}
			
	std::cerr << "(DD) ftFileMapper::initMappedChunks(): counted " << count << " filled chunks." << std::endl;
	std::cerr << "(DD) ftFileMapper::initMappedChunks(): printing: " << std::endl;

	print() ;

#ifdef DEBUG_FILEMAPPER
	consistencyTest();
#endif
}

bool ftFileMapper::moveChunk(uint32_t to_move, uint32_t new_place,FILE *fd_out)
{
	// Read the old chunk, write at the new place
	
	assert(to_move != new_place) ;

	fflush(fd_out) ;
#ifdef DEBUG_FILEMAPPER
	std::cerr << "(DD) ftFileMapper::moveChunk(): moving chunk " << to_move << " to place " << new_place << std::endl ;
#endif

	uint32_t new_place_size = (new_place == _mapped_chunks.size()-1)?(_file_size - (_mapped_chunks.size()-1)*_chunk_size) : _chunk_size ;
	uint32_t   to_move_size = ( to_move  == _mapped_chunks.size()-1)?(_file_size - (_mapped_chunks.size()-1)*_chunk_size) : _chunk_size ;

	uint32_t size = std::min(new_place_size,to_move_size) ;
	void *buff = malloc(size) ;

	if(buff == NULL)
	{
		std::cerr << "(EE) ftFileMapper::moveChunk(): cannot open temporary buffer. Out of memory??" << std::endl;
		return false ;
	}
	if(fseeko64(fd_out, to_move*(uint64_t)_chunk_size, SEEK_SET) != 0)
	{
		std::cerr << "(EE) ftFileMapper::moveChunk(): cannot fseek file at position " << to_move*_chunk_size << std::endl;
		return false ;
	}

	size_t rd ;
	if(size != (rd = fread(buff, 1, size, fd_out)))
	{
		std::cerr << "(EE) ftFileMapper::moveChunk(): cannot read from file" << std::endl;
		std::cerr << "(EE) errno = " << errno << std::endl;
		std::cerr << "(EE) feof  = " << feof(fd_out) << std::endl;
		std::cerr << "(EE) size  = " << size << std::endl;
		std::cerr << "(EE) rd    = " << rd << std::endl;
		return false ;
	}

	if(fseeko64(fd_out, new_place*(uint64_t)_chunk_size, SEEK_SET)!= 0)
	{
		std::cerr << "(EE) ftFileMapper::moveChunk(): cannot fseek file at position " << new_place*_chunk_size << std::endl;
		return false ;
	}

	if(1 != fwrite(buff, size, 1, fd_out))
	{
		std::cerr << "(EE) ftFileMapper::moveChunk(): cannot write to file" << std::endl;
		return false ;
	}
	free(buff) ;

	return true ;
}

void ftFileMapper::print() const
{
	std::cerr << "ftFileMapper:: [ " ;

	for(uint32_t i=0;i<_mapped_chunks.size();++i)
	{
		std::cerr << _mapped_chunks[i] << " " ;
	}

	std::cerr << "] - ffc = " << _data_chunk_ids.size() << " - [ ";
	
	for(uint32_t i=0;i<_data_chunk_ids.size();++i)
		std::cerr << _data_chunk_ids[i] << " " ;
	std::cerr << " ] " << std::endl;

}

bool ftFileMapper::consistencyTest() const
{
	for(uint32_t i=0;i<_data_chunk_ids.size();++i)
		if(!(_mapped_chunks[_data_chunk_ids[i]] == i))
		{
			std::cerr << "(EE) Consistency error in file mapper: _mapped_chunks[_data_chunk_ids[i]] != i, for i=" << i << std::endl;
			print() ;
			assert(false) ;
		}

	for(uint32_t i=0;i<_mapped_chunks.size();++i)
		if(!( (_mapped_chunks[i] == -1) || (_mapped_chunks[i] < _data_chunk_ids.size() && _data_chunk_ids[_mapped_chunks[i]] == i))) 
		{
			std::cerr << "(EE) Consistency error in file mapper: _mapped_chunks[i] != -1, and _mapped_chunks[i]=" << _mapped_chunks[i] << ". _data_chunk_ids.size()=" << _data_chunk_ids.size() << ", and _data_chunk_ids[_mapped_chunks[i]] == " << _data_chunk_ids[_mapped_chunks[i]] << ", for i=" << i << std::endl;
			print() ;
			assert(false) ;
		}
	return true ;
}

