#include "hashcache.h"

hash_cache create_cache(int nodes) {
	int i;
	hash_cache cache;

	cache.spacer = 3;
	cache.size = nodes;
	cache.nodes = (cache_node *)malloc(cache.size * cache.spacer * sizeof(cache_node));

	for(i = 0; i < cache.size * cache.spacer; ++i) {
		cache.nodes[i].source = -1;
		cache.nodes[i].dests = NULL;
	}

	return cache;
}


int get_cache(hash_cache cache, int source) {
	return (source * cache.spacer) % (cache.size * cache.spacer);
}


void add_dest(hash_cache cache, int source, int dest, char *path) {
	cache_node *source_node = get_cache_node(cache, source);

	if(source_node->source == -1) {
		source_node->source = source;
	}


	add_ll_cache(&(source_node->dests), dest, path);
}


cache_node *get_cache_node(hash_cache cache, int source) {
	int i = get_cache(cache, source);

	while(cache.nodes[i].source != -1 && cache.nodes[i].source != source) {
		i++;
	}

	return &(cache.nodes[i]);
}


char *is_cached(hash_cache cache, int source, int dest) {
	cache_node *source_node = get_cache_node(cache, source);

	return lookup_ll_cache(source_node->dests, dest);
}


void free_cache(hash_cache cache) {
	int i;

	for(i = 0; i < cache.size * cache.spacer; ++i) {
		if(cache.nodes[i].source != -1) {
			free_ll_cache(cache.nodes[i].dests);
		}
	}

	free(cache.nodes);
}

