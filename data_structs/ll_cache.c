#include "ll_cache.h"

void add_ll_cache(ll_cache **head, int new, char *path) {
	if(lookup_ll_cache(*head, new)) {
		return;
	}
	ll_cache *node = malloc(sizeof(ll_cache));
	node->dest = new;
	node->path = path;
	node->next = *head;
	*head = node;
}


char *lookup_ll_cache(ll_cache *head, int lookedup) {
	while(head != NULL) {
		if(head->dest == lookedup){
			return head->path;
		}
		head = head->next;
	}
	return NULL;
}


void free_ll_cache(ll_cache *head) {
	if(head == NULL) {
		return;
	}

	free_ll_cache(head->next);
	free(head->path);
	free(head);
}