#ifndef HASHGRAPH_H
#define HASHGRAPH_H

#include "ll_int.h"
#include <stdlib.h>
// #include <stdio.h> //print_graph 

typedef struct hash_node{
	int id;
	ll_int *edges;
} hash_node;

typedef struct hash_graph {
	int size;
	int spacer;
	hash_node *nodes;
} hash_graph;

// Takes the amount of nodes, and creates a hash graph with appropriate size
hash_graph create_graph(int nodes);

// retuns calculated id in the hash map 
int get_hash(hash_graph graph, int id);

// adds an edge to the graph from node id to another node id
void add_edge(hash_graph graph, int source, int dest);

// searches the graph for a node with id and returns it, or returns a node with an id of -1 upon failure
hash_node *get_node(hash_graph graph, int id);

// Print the edges in the graph... requires stdio
// void print_graph(hash_graph graph);

// frees all the nodes in the graph
void free_graph(hash_graph graph);

#endif