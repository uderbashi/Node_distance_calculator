#ifndef NANS_THREADS
#define NANS_THREADS

#include "DeLib/DeInO.h"
#include "data_structs/hashgraph.h"
#include "data_structs/hashcache.h"
#include "data_structs/queue_int.h"
#include "data_structs/req_queue.h"
#include "data_structs/bfs.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <stdio.h> // sprintf
#include <errno.h>

/*
** Why nan you may ask... well, because at home, she is the queen of threads, and the best jumpers
** I aslo couldn't come up with a better name to give
*/

typedef struct nans_t {
	int *term_flag;
	int output;
	int start;
	int max;
	int priority;
	hash_graph *graph;

	queue_int **pending;
	pthread_mutex_t *pending_mutex;
	sem_t *pending_sem;
} nans_t;

typedef struct threads_t {
	int id;
	int *term_flag;
	int output;
	int priority;
	hash_graph *graph;

	int *thread_status;
	pthread_mutex_t *status_mutex;

	req_queue **reqs;
	pthread_mutex_t *req_mutex;
	sem_t *req_sem;

	hash_cache *cache;
	pthread_mutex_t *cache_mutex;

	pthread_mutex_t *read_entry;
	int *read_count;
	pthread_mutex_t *read_mutex;
	int *write_count;
	pthread_mutex_t *write_mutex;
} threads_t;

int log_msg(char* msg, int fd);
void *unleash_nan(void *arguments);
void *unroll_thread(void *arguments);

#endif