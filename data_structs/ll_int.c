#include "ll_int.h"

void add_ll_int(ll_int **head, int new) {
	if(lookup_ll_int(*head, new)) {
		return;
	}
	ll_int *node = malloc(sizeof(ll_int));
	node->data = new;
	node->next = *head;
	*head = node;
}


int lookup_ll_int(ll_int *head, int lookedup) {
	while(head != NULL) {
		if(head->data == lookedup){
			return 1;
		}
		head = head->next;
	}

	return 0;
}


void free_ll_int(ll_int *head) {
	if(head == NULL) {
		return;
	}

	free_ll_int(head->next);
	free(head);
}