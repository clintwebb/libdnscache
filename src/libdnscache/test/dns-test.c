
#include <assert.h>
#include <dnscache.h>
#include <evactions.h>
#include <event.h>
#include <evdns.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct {
	struct event_base *main_event_base;
	action_pool_t  *actpool;
	dnscache_t *cache;
	char *domain;
} master_t;





// this function is called once the results of the DNS lookup have been obtained.
static void dns_callback(int result, char type, int count, int ttl, void *addresses, void *arg)
{
	in_addr_t *packed;
	master_t *master;

	assert(arg);
	master = arg;
	assert(master->domain);
	assert(master->cache);
	
	if (count > 0) {
		printf("dns_callback: host=%s, result=%d, count=%d, ttl=%d\n", master->domain, result, count, ttl);

		// address needs to be cast according to type... but what is the type in this case?
		packed = (in_addr_t *)addresses;
		dnscache_set(master->cache, master->domain, ttl, count, packed);
	}
	else {
		printf("dns_callback: resolve failed.\n");
	}
}


static void action_handler(action_t *action)
{
	master_t *master;
	int count;
	in_addr_t *addrs;
	int loop;
	
	assert(action);
	assert(action->shared);
	master = action->shared;

	assert(master->domain);
	printf("Checking cache for host: '%s'\n", master->domain);
	count = dnscache_get(master->cache, master->domain, &addrs);
	if (count > 0) {
		printf("Found %d results for %s", count, master->domain);
		action_pool_return(action);
	}
	else {
		printf("%s not found in cache.\n", master->domain);
		evdns_resolve_ipv4((const char *) master->domain, 0, dns_callback, master);
		action_reset(action);
	}

}

int main(int argc, char **argv) 
{
	master_t master;
	action_t *action;
	int loop;

	master.main_event_base = event_init();
	assert(master.main_event_base != NULL);
	if (evdns_init() != 0) {
		printf("Failed to initialise network objects.\n");
		exit(EXIT_FAILURE);
	}
	evdns_resolv_conf_parse(DNS_OPTIONS_ALL, "/etc/resolv.conf");

	master.cache = (dnscache_t *) malloc(sizeof(dnscache_t));
	dnscache_init(master.cache);
	printf("Created and initialised cache object.\n");

	master.actpool = (action_pool_t *) malloc(sizeof(action_pool_t));
	action_pool_init(master.actpool, master.main_event_base, &master);

	master.domain = "rhokz.com";

	for (loop=0; loop < 1; loop++) {
		action = action_pool_new(master.actpool);
		action_set(action, (1000 * loop), action_handler, NULL);
    action = NULL;
	}

	event_base_loop(master.main_event_base, 0);

	printf("test case complete.\n");
	evdns_shutdown(0);

	action_pool_free(master.actpool);
	free(master.actpool);
	master.actpool = NULL;

	dnscache_free(master.cache);
	free(master.cache);
	master.cache = NULL;

	return(0);
}
