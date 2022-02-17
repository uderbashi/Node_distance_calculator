#include "req_queue.h"

void add_req_queue(req_queue **head, request req, int fd) {
	req_queue *pointer = *head;
	req_queue *added = (req_queue*)malloc(sizeof(req_queue));
	
	added->req = req;
	added->fd = fd;
	added->next = NULL;

	if(*head == NULL) {
		*head = added;
	} else {
		while(pointer->next != NULL) {
			pointer = pointer->next;
		}
		pointer->next = added;
	}
}

req_queue get_req_queue(req_queue **head) {
	req_queue ret;
	req_queue *temp = *head;

	if(temp == NULL) {
		ret.req.source = 0;
		ret.req.dest = 0;
		ret.fd = 0;
		ret.next = NULL;
	} else {
		ret.req = temp->req;
		ret.fd = temp->fd;
		ret.next = NULL;
		
		*head = temp->next;
		free(temp);
	}

	return ret;
}

void free_req_queue(req_queue *head) {
	if(head == NULL) {
		return;
	}

	free_req_queue(head->next);
	free(head);
}