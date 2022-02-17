#include "hashgraph.h"

hash_graph create_graph(int nodes) {
	int i;
	hash_graph graph;

	graph.spacer = 3;
	graph.size = nodes;
	graph.nodes = (hash_node *)malloc(graph.size * graph.spacer * sizeof(hash_node));

	for(i = 0; i < graph.size * graph.spacer; ++i) {
		graph.nodes[i].id = -1;
		graph.nodes[i].edges = NULL;
	}

	return graph;
}


int get_hash(hash_graph graph, int id) {
	return (id* graph.spacer) % (graph.size * graph.spacer);
}


void add_edge(hash_graph graph, int source, int dest) {
	hash_node *source_node = get_node(graph, source);
	hash_node *dest_node = get_node(graph, dest);

	if(source_node->id == -1) {
		source_node->id = source;
	}

	if(dest_node->id == -1) {
		dest_node->id = dest;
	}

	add_ll_int(&(source_node->edges), dest);
}


hash_node *get_node(hash_graph graph, int id) {
	int i = get_hash(graph, id);

	while(graph.nodes[i].id != -1 && graph.nodes[i].id != id) {
		i++;
	}

	return &(graph.nodes[i]);
}


void free_graph(hash_graph graph) {
	int i;

	for(i = 0; i < graph.size * graph.spacer; ++i) {
		if(graph.nodes[i].id != -1) {
			free_ll_int(graph.nodes[i].edges);
		}
	}

	free(graph.nodes);
}



// void print_graph(hash_graph graph) {
// 	int i;
// 	ll_int *current;

// 	for(i = 0; i < graph.size * graph.spacer; ++i) {
// 		if(graph.nodes[i].id != -1) {
// 			current = graph.nodes[i].edges;

// 			while(current != NULL) {
// 				printf("%d->%d\n", graph.nodes[i].id, current->data);
// 				current = current->next;
// 			}
// 			printf("-----\n");
// 		}
// 	}
// }
