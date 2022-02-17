#ifndef BFS_H
#define BFS_H

#include "hashgraph.h"
#include "ll_int.h"
#include <string.h>
#include <stdio.h> //sprintf

typedef struct visited_nodes {
	hash_node *inner;
	int parent;
} visited_nodes;
	

// Bredth first search returns a string of the path NULL upon failure in the variable path
// THE USER HAS TO FREE THE STRING LATER 
char *BFS(hash_graph graph, int source, int dest);

int is_added(int id, visited_nodes vn[], int size);

int add_visited(visited_nodes vn[], int *added, hash_node *current, int parent);

char *get_path(visited_nodes vn[], int final);

int get_digits(int n);
#endif