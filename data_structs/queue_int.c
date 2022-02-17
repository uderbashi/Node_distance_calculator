#include "queue_int.h"

void add_queue_int(queue_int **head, int add) {
	queue_int *pointer = *head;
	queue_int *added = (queue_int*)malloc(sizeof(queue_int));
	added->data = add;
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

int get_queue_int(queue_int **head) {
	queue_int *temp = *head;
	int ret;

	if(temp == NULL) {
		return 0;
	} else {
		ret = temp->data;
		
		*head = temp->next;
		free(temp);
	}

	return ret;
}

void free_queue_int(queue_int *head) {
	if(head == NULL) {
		return;
	}

	free_queue_int(head->next);
	free(head);
}