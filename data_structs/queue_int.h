#ifndef QUEUE_INT_H
#define QUEUE_INT_H

#include <stdlib.h>

typedef struct queue_int {
	double data;
	struct queue_int *next;
}queue_int;

void add_queue_int(queue_int **head, int added);

int get_queue_int(queue_int **head);

void free_queue_int(queue_int *head);
#endif