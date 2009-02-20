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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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



typedef struct {
	char *host;
	int expires;
	in_addr_t *list;
	int count;
	void *next, *prev;			// in the lru list.
} dnsentry_t;



//-----------------------------------------------------------------------------
// Initialise the cache structure. will be empty.
static void dnsentry_init(dnsentry_t *entry)
{
	assert(entry);

	entry->host = NULL;
	entry->expires = 0;
	
	entry->list = NULL;
	entry->count = 0;
	
	entry->next = NULL;
	entry->prev = NULL;
}


//-----------------------------------------------------------------------------
// Prepare the cache structure to be de-allocated.  Free's all memory that was
// created for it.
void dnsentry_free(dnsentry_t *entry)
{
	assert(entry);

	if (entry->host) {
		free(entry->host);
		entry->host = NULL;
	}

	assert((entry->count == 0 && entry->list == NULL) || (entry->count > 0 && entry->list));
	if (entry->list) {
		free(entry->list);
		entry->list = NULL;
	}
}



/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**   DNS CACHE - public facing functions.                                  **/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/



//-----------------------------------------------------------------------------
// Initialise the cache structure. will be empty.
void dnscache_init(dnscache_t *cache)
{
	assert(cache);

	cache->list = NULL;
	cache->entries = 0;
	
	cache->lru = NULL;
	cache->mru = NULL;

	cache->current = 0;
	cache->maxmem = 0;
}

//-----------------------------------------------------------------------------
// Prepare the cache structure to be de-allocated.  Free's all memory that was
// created for it.
void dnscache_free(dnscache_t *cache)
{
	assert(cache);
	assert((cache->lru == NULL && cache->mru == NULL) || (cache->lru && cache->mru));
	assert(cache->current >= 0);
	assert(cache->maxmem >= 0);
	assert((cache->lru == NULL && cache->current == 0) || (cache->lru && cache->current > 0));
	assert((cache->entries == 0 && cache->list == NULL) || (cache->entries > 0 && cache->list));

	while (cache->entries > 0) {
		cache->entries --;
		if (cache->list[cache->entries]) {
			dnsentry_free(cache->list[cache->entries]);
			cache->list[cache->entries] = NULL;
		}
	}
	if (cache->list) {
		free(cache->list);
		cache->list = NULL;
	}
	assert(cache->entries == 0);
	assert(cache->list == NULL);

	cache->lru = NULL;
	cache->mru = NULL;
}


//-----------------------------------------------------------------------------
// Set the maximum amount of memory that will be allocated to the cache.  If
// current memory usage is greater than the new limit, it will not reduce
// usage.  That can only be done with the 'pack' function.  In this situation,
// it just means that no new objects will be created.
void dnscache_setmemlimit(dnscache_t *cache, int limit)
{
	assert(cache);
	assert(limit >= 0);

	assert((cache->lru == NULL && cache->mru == NULL) || (cache->lru && cache->mru));
	assert(cache->current >= 0);
	assert(cache->maxmem >= 0);
	assert((cache->lru == NULL && cache->current == 0) || (cache->lru && cache->current > 0));
	assert((cache->entries == 0 && cache->list == NULL) || (cache->entries > 0 && cache->list));

	cache->maxmem = limit;
}


//-----------------------------------------------------------------------------
// Load cache entries from a specified file.  File is a text based csv.
void dnscache_load(dnscache_t *cache, char *filepath)
{
	assert(cache);
	assert(filepath);
	assert(0);
}

//-----------------------------------------------------------------------------
// Save the cached entries to a file so it can be loaded later.
void dnscache_save(dnscache_t *cache, char *filepath)
{
	assert(cache);
	assert(filepath);
	assert(0);
}


//-----------------------------------------------------------------------------
// We have a large sorted array of pointers, which we will do a binary search
// on to find the specified host entry.
//
// TODO: We could check the MRU entry to see if it is the one we are looking
//       for.  We could check the first few in the mru linked-list.  This would
//       save having to search the entire list for entries that had just been
//       looked at.  Especially when we do a get and then do a set (for expired
//       cases).
static int dnscache_locate
	(dnscache_t *cache, char *host)
{
	int index = -1;
	int min, max, curr;
	int check;
	dnsentry_t *tmp;

	assert(cache);
	assert(host);

	min = 0;
	max = cache->entries-1;
	
	while (index < 0 && (min <= max)) {
	
		curr = min + ((max-min)/2);
		assert(cache->list);
		assert(cache->list[curr]);

		tmp = cache->list[curr];
		assert(tmp->host);
		check = strcasecmp(tmp->host, host);
		if (check == 0)			{ index = curr; }
		else if (check < 0)	{ max = curr - 1; }
		else								{ min = curr + 1; }
	}

	return(index);
}

//-----------------------------------------------------------------------------
// we need to update this entries LRU.
static void dnscache_setmru(dnscache_t *cache, int index)
{
	dnsentry_t *next, *prev, *tmp;
	dnsentry_t *entry;

	assert(cache);
	assert(cache->list);
	assert(index < cache->entries);
	assert(index >= 0);

	assert(cache->lru);
	assert(cache->mru);

	entry = cache->list[index];
	assert(entry);

	prev = entry->prev;
	if (cache->lru == entry && prev != NULL) {
		cache->lru = prev;
		prev->next = NULL;
	}
	if (cache->mru != entry) {
		next = entry->next;

		tmp = cache->mru;
		cache->mru = entry;

		assert(prev);
		prev->next = next;

		if (next) {
			next->prev = prev;
		}

		entry->prev = NULL;
		entry->next = tmp;
		tmp->prev = entry;
	}
}


//-----------------------------------------------------------------------------
// Go thru the list and find the index of the object that is actually the LRU
// object.  We will need to iterate through the list to find it.
static int dnscache_getlruindex(dnscache_t *cache)
{
	int i;
	int index;

	assert(cache);

	if (cache->lru == NULL) {
		assert(cache->mru == NULL);
		index = -1;
	}
	else {
		for (i=0; i<cache->entries && index < 0; i++) {
			if (cache->list == cache->lru) {
				index = 1;
			}
		}
		assert(index >= 0);
	}
	
	assert(index < cache->entries);
	return(index);
}


//-----------------------------------------------------------------------------
// We need to look at the entry specified by 'index'.  We then need to go
// through the list to determine where the best spot would be for it to be
// inserted.  Then it inserts it in.
static int dnscache_sortentry
	(dnscache_t *cache, int index)
{
	int i, j, result;
	int sorted = -1;
	dnsentry_t *entry, *tmp, *swap;
	
	assert(cache);
	assert(index >= 0);
	assert(index < cache->entries);

	if (cache->mru != cache->lru) {
		entry = cache->list[index];
		assert(entry);

		for (i=0; i < cache->entries && sorted < 0; i++) {
			tmp = cache->list[i];
			assert(tmp);

			result = strcmp(tmp->host, entry->host);
			if (result == 0) {
				assert(i == index);
			}
			else if (result > 0) {
				// we found one that is greater than what we are looking for, so we need to insert before it.
				if (i < index) {
					// the one we are moving is further in the list.
					sorted = i;
					for (j=index; j>i; j--) {
						assert(j>=0);
						assert((j-1)>=i);
						cache->list[j] = cache->list[j-1];
					}
					cache->list[i] = entry;
				}
				else {
					// the one we are moving is back.
					assert (i > index);
					sorted = i-1;
					assert(sorted >= 0);
					for (j=index; j<sorted; j++) {
						cache->list[j] = cache->list[j+1];
					}
					cache->list[sorted] = entry;
				}
			}
		}
	}

	assert((sorted < 0 && cache->mru == cache->lru) || (sorted > 0 && cache->mru != cache->lru && cache->mru && cache->lru));

	return(sorted);
}


//-----------------------------------------------------------------------------
// Check the cache for an entry.
int dnscache_get
	(dnscache_t *cache, char *host, in_addr_t **addrlist)
{
	dnsentry_t *entry;
	int index;
	
	assert(cache);
	assert(host);
	assert(addrlist);

	index = dnscache_locate(cache, host);
	if (index < 0) {
		return(0);
	}
	else {
		assert(index < cache->entries);
		assert(cache->list);
		assert(cache->list[index]);

		dnscache_setmru(cache, index);

		entry = cache->list[index];
	
		// we have the entry we were looking for, now we need to make sure this entry hasn't expired.
		assert(0);

		assert(entry->list);
		assert(entry->count > 0);
		*addrlist = entry->list;
		return(entry->count);
	}
}

//-----------------------------------------------------------------------------
// Set details for an entry (replace if entry already exists)
void dnscache_set(
	dnscache_t *cache,
	char *host,
	int ttl,
	int count,
	in_addr_t *addrlist)
{
	int index;
	int sorted;
	dnsentry_t *entry = NULL, *tmp;
	
	assert(cache);
	assert(host);
	assert(count > 0);
	assert(addrlist);
	
	index = dnscache_locate(cache, host);
	if (index < 0) {
		// entry isn't in there yet, so we need to add it.

		// check that we haven't reached our memmax limit.
		if (cache->maxmem == 0 || cache->current < cache->maxmem) {
		
			// allocate dnsentry...
			entry = (dnsentry_t *) malloc(sizeof(dnsentry_t));
			cache->current += sizeof(dnsentry_t);
			dnsentry_init(entry);

			// increase size of master list.
			cache->list = (void **) realloc(cache->list, sizeof(void *) * (cache->entries + 1));
			index = cache->entries;
			cache->list[index] = entry;
			cache->current += sizeof(void *);

			// Set this new entry as the lru.
			if (cache->lru == NULL) {
				assert(cache->mru == NULL);
				cache->lru = entry;
				cache->mru = entry;
			}
			else {
				tmp = cache->lru;
				assert(tmp->next == NULL);
				tmp->next = entry;
				cache->lru = entry;
			}
		}
		else {
			index = dnscache_getlruindex(cache);
			entry = cache->list[index];
		}
		assert(index >= 0);
		assert(index < cache->entries);
		assert(entry != NULL);

		// resize its memory (keeping track of memory changes)
		if (entry->host == NULL) {
			cache->current += (strlen(host) + 1);
			entry->host = (char *) malloc(strlen(host)+1);
			assert(entry->host);
			strcpy(entry->host, host);
		}
		else {
			cache->current -= strlen(entry->host);
			cache->current += strlen(host);
			entry->host = (char *) realloc(entry->host, strlen(host)+1);
			assert(entry->host);
			strcpy(entry->host, host);
		}
		
		// find the appropriate spot in the master list.
		sorted = dnscache_sortentry(cache, index);
	}
	else {
		entry = cache->list[index];
	}

	assert(index >= 0);
	assert(index < cache->entries);
	assert(cache->list);
	assert(cache->list[index]);
	assert(entry);
	assert(entry == cache->list[index]);

	dnscache_setmru(cache, index);

	// resize the addrs list.
	assert((entry->list == NULL && entry->count == 0) || (entry->list && entry->count > 0));
	if (entry->count > 0 && entry->count != count) {
		free(entry->list);
		cache->current -= (sizeof(in_addr_t) * entry->count);
	}
		
	entry->list = (in_addr_t *) malloc(sizeof(in_addr_t) * count);
	entry->count = count;
	cache->current += (sizeof(in_addr_t) * entry->count);

	// copy the addresses,
	memcpy(entry->list, addrlist, sizeof(in_addr_t)*count);
	
	// set expire time.
	assert(0);
}


//-----------------------------------------------------------------------------
// where possible reduce memory that is used by the cache.
// Also remove expired items.
void dnscache_pack(dnscache_t *cache)
{
	assert(cache);
	assert(0);
}




