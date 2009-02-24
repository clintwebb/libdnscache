
#include <assert.h>
#include <dnscache.h>
#include <evdns.h>
#include <stdio.h>
#include <stdlib.h>




int main(int argc, char **argv) 
{
	dnscache_t *cache;
	char *domain;
	int loop;
	in_addr_t addrlist[5];
	int count;
	in_addr_t *addr;

	cache = (dnscache_t *) malloc(sizeof(dnscache_t));
	dnscache_init(cache);
	printf("Created and initialised cache object.\n");
	
	addrlist[0] = 4444;
	addrlist[1] = 5555;
	addrlist[2] = 6666;
	printf("Adding 'zoogle.com' to the cache [%u, %u, %u].\n", addrlist[0], addrlist[1], addrlist[2]);
	dnscache_set(cache, "zoogle.com", 600, 3, addrlist);
	printf("\n");
	
	addrlist[0] = 1234;
	printf("Adding 'rhokz.com' to the cache [%u].\n", addrlist[0]);	
	dnscache_set(cache, "rhokz.com", 600, 1, addrlist);
	printf("\n");

	addrlist[0] = 2345;
	addrlist[1] = 3456;
	printf("Adding 'google.com' to the cache [%u, %u].\n", addrlist[0], addrlist[1]);
	dnscache_set(cache, "google.com", 5, 2, addrlist);
	printf("\n");
	
	sleep(10);
	
	count = dnscache_get(cache, "rhokz.com", &addr);
	printf("Get 'rhokz.com' from the cache.  count=%d, [%u].\n\n", count, addr[0]);
	
	count = dnscache_get(cache, "google.com", &addr);
	printf("Get 'google.com' from the cache.  count=%d, [%u, %u].\n\n", count, addr[0], addr[1]);
 
	count = dnscache_get(cache, "zoogle.com", &addr);
	printf("Get 'zoogle.com' from the cache.  count=%d, [%u, %u, %u].\n\n", count, addr[0], addr[1], addr[2]);
	
	printf("test case complete.\n");

	dnscache_free(cache);
	free(cache);
	cache = NULL;

	return(0);
}
