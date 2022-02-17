#ifndef LL_INT_H
#define LL_INT_H

#include <stdlib.h>

typedef struct ll_int {
	int data;
	struct ll_int *next;
}ll_int;

// adds a node to the list only if a node with a similar name doesn't exist 
void add_ll_int(ll_int **head, int new);

// returns 1 if a node with the same name exists and 0 if not 
int lookup_ll_int(ll_int *head, int lookedup);

// frees all the nodes in the list
void free_ll_int(ll_int *head);

#endif