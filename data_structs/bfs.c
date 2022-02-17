#include "bfs.h"

char *BFS(hash_graph graph, int source, int dest) {
	visited_nodes vn[graph.size];
	hash_node *current;
	ll_int *edge;
	int visited = 0, added = 0;
	char *ret;

	current = get_node(graph, source);

	if(current->id == -1) {
		ret = (char*) malloc(sizeof(char));
		ret[0] = '\0';
		return ret;
	}

	vn[0].inner = current;
	vn[0].parent = 0;
	added++;

	if(source == dest) {
		return get_path(vn, added);
	}

	do {
		current = vn[visited].inner;
		edge = current->edges;
		while(edge != NULL) {
			if(add_visited(vn, &added, get_node(graph, edge->data), visited) == 1 && edge->data == dest) {
				return get_path(vn, added);
			}

			edge = edge->next;
		}
		visited++;
	} while(visited < graph.size && visited < added);

	ret = (char*) malloc(sizeof(char));
	ret[0] = '\0';
	return ret;
}


int is_added(int id, visited_nodes vn[], int size) {
	int i;
	for(i = 0; i < size; ++i) {
		if(vn[i].inner->id == id) {
			return 1;
		}
	}

	return 0;
}


int add_visited(visited_nodes vn[], int *added, hash_node *current, int parent) {
	if(is_added(current->id, vn, *added) == 1) {
		return 0;
	}

	vn[*added].inner = current;
	vn[*added].parent = parent;

	*added = *added + 1;
	return 1;
}


char *get_path(visited_nodes vn[], int final) {
	char *path;
	int nodes[final];
	int len = 1;
	int current = final - 1;
	int i;

	for(i = 0; i < final; i++) {
		nodes[i] = vn[current].inner->id;
		len += (2 + get_digits(vn[current].inner->id));

		if(vn[current].inner->id == vn[vn[current].parent].inner->id) {
			break;
		}

		current = vn[current].parent;
	}
	path = (char *)malloc(len * sizeof(char));

	len = 0;
	while(i >= 0) {
		len += sprintf(path + len, "%d", nodes[i]);
		if(i != 0) {
			len += sprintf(path + len, "->");
		}
		--i;
	}


	return path;
}


int get_digits(int n) {
	int digits = 0;

	while(n != 0) {
		n /= 10;
		digits++;
	}

	return digits;
}