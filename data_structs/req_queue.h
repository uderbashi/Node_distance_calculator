#ifndef REQ_QUEUE_H
#define REQ_QUEUE_H

#include <stdlib.h>

typedef struct request {
	int source;
	int dest;
} request;

typedef struct req_queue {
	int fd;
	request req;
	struct req_queue *next;
}req_queue;

void add_req_queue(req_queue **head, request req, int fd);

req_queue get_req_queue(req_queue **head);

void free_req_queue(req_queue *head);
#endif