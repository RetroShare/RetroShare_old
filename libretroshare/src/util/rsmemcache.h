/*
 * libretroshare/src/util: rsmemcache.h
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
#ifndef RS_UTIL_MEM_CACHE
#define RS_UTIL_MEM_CACHE

#include <map>
#include <time.h>
#include <iostream>
#include <inttypes.h>
#include <string>

/************************************************************************************/
/************************************************************************************/
/************************************************************************************/

/* Generic Memoory Cache 
 *
 * This is probably crude and crap to start with.
 * Want Least Recently Used (LRU) discard policy, without having to search whole cache.
 * Use two maps:
 *   - mDataMap[key] => data.
 *   - mLruMap[AccessTS] => key (multimap)
 */

#define DEFAULT_MEM_CACHE_SIZE 100

template<class Key, class Value> class RsMemCache
{
	public:

	RsMemCache(uint32_t max_size = DEFAULT_MEM_CACHE_SIZE, std::string name = "UnknownMemCache")
	:mDataCount(0), mMaxSize(max_size), mName(name) 
	{ 
		clearStats();
		return;
	}

	bool is_cached(const Key &key) const;
	bool fetch(const Key &key, Value &data);
	Value &ref(const Key &key);	// like map[] installs empty one if non-existent.
	bool store(const Key &key, const Value &data);

	bool resize(); // should be called periodically to cleanup old entries.

	private:

	bool update_lrumap(const Key &key, time_t old_ts, time_t new_ts);
	bool discard_LRU(int count_to_clear);

	// internal class.
	class cache_data
	{
		public:
		cache_data() { return; }
		cache_data(Key in_key, Value in_data, time_t in_ts)
		:key(in_key), data(in_data), ts(in_ts) { return; }
		Key key;
		Value data;
		time_t ts;
	};


        std::map<Key, cache_data > mDataMap;
        std::multimap<time_t, Key> mLruMap;
        uint32_t mDataCount;
	uint32_t mMaxSize;
	std::string mName;

	// some statistics.
        void printStats(std::ostream &out);
	void clearStats();

	mutable uint32_t mStats_inserted;
	mutable uint32_t mStats_dropped;
	mutable uint32_t mStats_iscached;
	mutable uint32_t mStats_cachemiss;
	mutable uint32_t mStats_access;
	mutable uint32_t mStats_accessmiss;
};


template<class Key, class Value> bool RsMemCache<Key, Value>::is_cached(const Key &key) const
{
	typename std::map<Key,cache_data>::const_iterator it;
	it = mDataMap.find(key);
	if (it == mDataMap.end())
	{
		std::cerr << "RsMemCache::is_cached(" << key << ") false";
		std::cerr << std::endl;

		mStats_cachemiss++;
		return false;
	}
	std::cerr << "RsMemCache::is_cached(" << key << ") false";
	std::cerr << std::endl;
	mStats_iscached++;
	return true;
	
}


template<class Key, class Value> bool RsMemCache<Key, Value>::fetch(const Key &key, Value &data)
{
	printStats(std::cerr);
	typename std::map<Key, cache_data>::iterator it;
	it = mDataMap.find(key);
	if (it == mDataMap.end())
	{
		std::cerr << "RsMemCache::fetch(" << key << ") false";
		std::cerr << std::endl;

		mStats_accessmiss++;
		return false;
	}
	
	std::cerr << "RsMemCache::fetch(" << key << ") OK";
	std::cerr << std::endl;

	data = it->second.data;

	/* update ts on data */
        time_t old_ts = it->second.ts;
        time_t new_ts = time(NULL);
        it->second.ts = new_ts;

        update_lrumap(key, old_ts, new_ts);

	mStats_access++;
	return true;
}


template<class Key, class Value> Value &RsMemCache<Key, Value>::ref(const Key &key)
{
	printStats(std::cerr);
	typename std::map<Key, cache_data>::iterator it;
	it = mDataMap.find(key);
	if (it == mDataMap.end())
	{
		std::cerr << "RsMemCache::ref(" << key << ") ERROR missing Key inserting Empty Data in LRU slot";
		std::cerr << std::endl;

		// insert operation.
		time_t new_ts = 0;
		Value data;
		mDataMap[key] = cache_data(key, data, new_ts);
        	mDataCount++;

        	update_lrumap(key, 0, new_ts);
		it = mDataMap.find(key);

		mStats_accessmiss++;
	}
	else
	{
		std::cerr << "RsMemCache::ref(" << key << ") OK";
		std::cerr << std::endl;

		/* update ts on data */
        	time_t old_ts = it->second.ts;
        	time_t new_ts = time(NULL);
        	it->second.ts = new_ts;

        	update_lrumap(key, old_ts, new_ts);

		mStats_access++;
	}
	return it->second.data;
}

template<class Key, class Value> bool RsMemCache<Key, Value>::store(const Key &key, const Value &data)
{
	std::cerr << "RsMemCache::store()";
	std::cerr << std::endl;
	printStats(std::cerr);

	/* update lrumap entry */
        time_t old_ts = 0;
        time_t new_ts = time(NULL);

	// For consistency
	typename std::map<Key, cache_data>::const_iterator it;
	it = mDataMap.find(key);
	if (it != mDataMap.end())
	{
		// ERROR.
		std::cerr << "RsMemCache::store() WARNING overriding existing entry";
		std::cerr << std::endl;

		old_ts = it->second.ts;
	}
	else
	{
        	mDataCount++;
	}

	mDataMap[key] = cache_data(key, data, new_ts);

        update_lrumap(key, old_ts, new_ts);

	mStats_inserted++;
	return true;
}


template<class Key, class Value> bool RsMemCache<Key, Value>::update_lrumap(const Key &key, time_t old_ts, time_t new_ts)
{
	if (old_ts == 0)
	{
		std::cerr << "p3IdService::locked_cache_update_lrumap(" << key << ") just insert!";
		std::cerr << std::endl;

		/* new insertion */
		mLruMap.insert(std::make_pair(new_ts, key));
		return true;
	}

	/* find old entry */
	typename std::multimap<time_t, Key>::iterator mit;
	typename std::multimap<time_t, Key>::iterator sit = mLruMap.lower_bound(old_ts);
	typename std::multimap<time_t, Key>::iterator eit = mLruMap.upper_bound(old_ts);

        for(mit = sit; mit != eit; mit++)
	{
		if (mit->second == key)
		{
			mLruMap.erase(mit);
			std::cerr << "p3IdService::locked_cache_update_lrumap(" << key << ") rm old";
			std::cerr << std::endl;

			if (new_ts != 0) // == 0, means remove.
			{	
				std::cerr << "p3IdService::locked_cache_update_lrumap(" << key << ") added new_ts";
				std::cerr << std::endl;
				mLruMap.insert(std::make_pair(new_ts, key));
			}
			return true;
		}
	}
	std::cerr << "p3IdService::locked_cache_update_lrumap(" << key << ") ERROR";
	std::cerr << std::endl;

	return false;
}

template<class Key, class Value> bool RsMemCache<Key, Value>::resize()
{
	std::cerr << "RsMemCache::resize()";
	std::cerr << std::endl;
	printStats(std::cerr);

	int count_to_clear = 0;
	{
		// consistency check.
		if ((mDataMap.size() != mDataCount) ||
			(mLruMap.size() != mDataCount))
		{
			// ERROR.
			std::cerr << "RsMemCache::resize() CONSISTENCY ERROR";
			std::cerr << std::endl;
		}
	
		if (mDataCount > mMaxSize)
		{
			count_to_clear = mDataCount - mMaxSize;
			std::cerr << "RsMemCache::resize() to_clear: " << count_to_clear;
			std::cerr << std::endl;
		}
	}

	if (count_to_clear > 0)
	{
		discard_LRU(count_to_clear);
	}
	return true;
}



template<class Key, class Value> bool RsMemCache<Key, Value>::discard_LRU(int count_to_clear)
{
	while(count_to_clear > 0)
	{
		typename std::multimap<time_t, Key>::iterator mit = mLruMap.begin();
		if (mit != mLruMap.end())
		{
			Key key = mit->second;
			mLruMap.erase(mit);

			/* now clear from real cache */
			//std::map<Key, cache_data<Key, Value> >::iterator it;
			typename std::map<Key, cache_data>::iterator it;
			it = mDataMap.find(key);
			if (it == mDataMap.end())
			{
				// ERROR
				std::cerr << "RsMemCache::discard_LRU(): ERROR Missing key: " << key;
				std::cerr << std::endl;
				return false;
			}
			else
			{
				std::cerr << "RsMemCache::discard_LRU() removing: " << key;
				std::cerr << std::endl;
				mDataMap.erase(it);
				mDataCount--;
				mStats_dropped++;
			}
		}
		else
		{
			// No More Data, ERROR.
			std::cerr << "RsMemCache::discard_LRU(): INFO more more cache data";
			std::cerr << std::endl;
			return true;
		}
		count_to_clear--;
	}
	return true;
}

// These aren't templated functions.
template<class Key, class Value> void RsMemCache<Key, Value>::printStats(std::ostream &out)
{
	typename std::multimap<time_t, Key>::iterator mit = mLruMap.begin();
	time_t age = 0;
	if (mit != mLruMap.end())
	{
		age = time(NULL) - mit->first;
	}
	
	out << "RsMemCache<" << mName << ">::printStats() Size: " << mDataCount << " Size2: " << mDataMap.size() << " Size3: " << mLruMap.size() << " MaxSize: " << mMaxSize << " LRU Age: " << age;
	out << std::endl;

	out << "\tInsertions: " << mStats_inserted << " Drops: " << mStats_dropped;
	out << std::endl;

	out << "\tCache Hits: " << mStats_iscached << " Misses: " << mStats_cachemiss;
	out << std::endl;

	out << "\tAccess Hits: " << mStats_access << " Misses: " << mStats_accessmiss;
	out << std::endl;
}

template<class Key, class Value> void RsMemCache<Key, Value>::clearStats()
{
	mStats_inserted = 0;
	mStats_dropped = 0;
	mStats_iscached = 0;
	mStats_cachemiss = 0;
	mStats_access = 0;
	mStats_accessmiss = 0;
}




#endif // RS_UTIL_MEM_CACHE
