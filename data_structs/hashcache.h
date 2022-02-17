#ifndef HASHCACHE_H
#define HASHCACHE_H

#include "ll_cache.h"
#include <stdlib.h>

typedef struct cache_node{
	int source;
	ll_cache *dests;
} cache_node;

typedef struct hash_cache {
	int size;
	int spacer;
	cache_node *nodes;
} hash_cache;

// Takes the amount of nodes, and creates a hash cache with appropriate size
hash_cache create_cache(int nodes);

// retuns calculated source in the hash map 
int get_cache(hash_cache cache, int source);

// adds an entry to the cache from node source to another node source
void add_dest(hash_cache cache, int source, int dest, char *path);

// searches the cache for a node with source and returns it, or returns a node with an source of -1 upon failure
cache_node *get_cache_node(hash_cache cache, int source);

// looks up if a source pair has been cached, returns the path if successful or NULL if failed
char *is_cached(hash_cache cache, int source, int dest);

// frees all the nodes in the cache
void free_cache(hash_cache cache);

#endif