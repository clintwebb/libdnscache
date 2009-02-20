/*
	libdnscache
	(c) Copyright 2009, Hyper-Active Systems, Australia
	http://hyper-active.com.au/libdnscache

	Contact:
		Clinton Webb
		webb.clint@gmail.com

	This small library intended to provide a dnscache that can be used to store looked up DNS values.
	It does not lookup DNS entries itself.

	It is released under GPL v2 or later license.  Details are included in the LICENSE file.
*/

#ifndef __DNSCACHE_H
#define __DNSCACHE_H

#include <netinet/in.h>



typedef struct {
	void **list;			// list of objects...
	int entries;			// number of objects in the list.
	void *lru, *mru;	// least recently used, and most recently used.

	int current;			// current amount of memory used.
	int maxmem;				// max amount of memory to use... when current reaches this limit, start using the lru.
} dnscache_t;


// 	Initialise the cache structure. will be empty.
void dnscache_init(dnscache_t *cache);

// 	Prepare the cache structure to be de-allocated.  Free's all memory that was created for it.
void dnscache_free(dnscache_t *cache);

// Set the maximum amount of memory that will be used by the cache (not including memory chunk over-head.)
void dnscache_setmemlimit(dnscache_t *cache, int limit);

// Load cache entries from a specified file.  File is a text based csv.
void dnscache_load(dnscache_t *cache, char *filepath);

// Save the cached entries to a file so it can be loaded later.
void dnscache_save(dnscache_t *cache, char *filepath);

// Check the cache for an entry.
int dnscache_get(dnscache_t *cache, char *host, in_addr_t **addrlist);

// Set details for an entry (replace if entry already exists)
void dnscache_set(dnscache_t *cache, char *host, int ttl, int count, in_addr_t *addrlist);

// 	pack - where possible reduce memory that is used by the cache.  remove expired items
void dnscache_pack(dnscache_t *cache);



#endif
