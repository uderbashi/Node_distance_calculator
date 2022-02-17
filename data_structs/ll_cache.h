#ifndef LL_CACHE_H
#define LL_CACHE_H

#include <stdlib.h>
#include <string.h>

typedef struct ll_cache {
	int dest;
	char *path;
	struct ll_cache *next;
}ll_cache;

// adds a node to the list only if a node with the same destination doesn't exist 
void add_ll_cache(ll_cache **head, int new, char *path);

// returns 1 and the path if a node with the same destination exists and 0 if not 
char *lookup_ll_cache(ll_cache *head, int lookedup);

// frees all the nodes in the list
void free_ll_cache(ll_cache *head);

#endif