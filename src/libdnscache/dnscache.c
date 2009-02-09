/*

	libevhttpget
	(c) Copyright Hyper-Active Systems, Australia

	Contact:
		Clinton Webb
		webb.clint@gmail.com

*/


#include "dnscache.h"

#include <assert.h>
// #include <fcntl.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// 
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <netdb.h>
// 
// #include <event.h>
// #include <evdns.h>
// #include <parseurl.h>



//-----------------------------------------------------------------------------
// Initialise the cache structure. will be empty.
void dnscache_init(dnscache_t *cache)
{
	assert(cache);
}

//-----------------------------------------------------------------------------
// Prepare the cache structure to be de-allocated.  Free's all memory that was
// created for it.
void dnscache_free(dnscache_t *cache)
{
	assert(cache);
}

//-----------------------------------------------------------------------------
// Load cache entries from a specified file.  File is a text based csv.
void dnscache_load(dnscache_t *cache, char *filepath)
{
	assert(cache);
	assert(filepath);
}

//-----------------------------------------------------------------------------
// Save the cached entries to a file so it can be loaded later.
void dnscache_save(dnscache_t *cache, char *filepath)
{
	assert(cache);
	assert(filepath);
}

//-----------------------------------------------------------------------------
// Check the cache for an entry.
int dnscache_get(dnscache_t *cache, char *host, in_addr_t **addrlist)
{
	assert(cache);
	assert(host);
	assert(addrlist);
}

//-----------------------------------------------------------------------------
// Set details for an entry (replace if entry already exists)
void dnscache_set(dnscache_t *cache, char *host, int ttl, int count, in_addr_t *addrlist)
{
	assert(cache);
	assert(host);
	assert(count > 0);
	assert(addrlist);
}


//-----------------------------------------------------------------------------
// where possible reduce memory that is used by the cache.
// Also remove expired items.
void dnscache_pack(dnscache_t *cache)
{
	assert(cache);
}




