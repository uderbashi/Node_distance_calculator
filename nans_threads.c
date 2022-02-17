#include "nans_threads.h"

int log_msg(char* msg, int fd) {
	char full[strlen(msg) + 32];
	time_t ttime;
	struct tm *tm;
	int written;

	ttime = time(NULL);
	tm=localtime(&ttime);

	sprintf(full,"%04d-%02d-%02d-%02d-%02d-%02d\t%s", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, msg);

	written = de_write(fd, full);

	// de_write returns a negative value for errors which we can send on
	return written;
}


void *unleash_nan(void *arguments) {
	nans_t *args = (nans_t*) arguments; // not to cast at every line
	pthread_t threads[args->max];
	int current_threads = args->start;
	int current_client;
	request current_request;
	char msg[128];
	
	int thread_status[args->max]; // 0 free, 1 busy
	pthread_mutex_t status_mutex = PTHREAD_MUTEX_INITIALIZER;

	hash_cache cache = create_cache(args->graph->size);
	pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

	req_queue *reqs = NULL;
	pthread_mutex_t req_mutex = PTHREAD_MUTEX_INITIALIZER;
	sem_t req_sem;

	pthread_mutex_t read_entry= PTHREAD_MUTEX_INITIALIZER;
	int read_count = 0;
	pthread_mutex_t read_mutex= PTHREAD_MUTEX_INITIALIZER;
	int write_count = 0;
	pthread_mutex_t write_mutex= PTHREAD_MUTEX_INITIALIZER;

	threads_t thread_args[args->max];
	int i, j, k;
	void *res;

	if(sem_init(&req_sem, 0, 0) < 0) {
		log_msg("ERROR CREATING SEMAPHORE IN NAN\n", args->output);
		kill(getpid(), SIGINT);
		pthread_exit(0);
	}

	for(i = 0; i < args->max; ++i) {
		thread_status[i] = 0;
		thread_args[i].id = i;
		thread_args[i].term_flag = args->term_flag;
		thread_args[i].output = args->output;
		thread_args[i].priority = args->priority;
		thread_args[i].graph = args->graph;
		thread_args[i].thread_status = thread_status;
		thread_args[i].status_mutex = &status_mutex;
		thread_args[i].cache = &cache;
		thread_args[i].cache_mutex = &cache_mutex;
		thread_args[i].reqs = &reqs;
		thread_args[i].req_mutex = &req_mutex;
		thread_args[i].req_sem = &req_sem;
		thread_args[i].read_entry = &read_entry;
		thread_args[i].read_count = &read_count;
		thread_args[i].read_mutex = &read_mutex;
		thread_args[i].write_count = &write_count;
		thread_args[i].write_mutex = &write_mutex;
	}

	for(i = 0; i< args->start; ++i) {
		if(pthread_create(&(threads[i]), NULL, unroll_thread, (void *)&thread_args[i]) != 0) {
			for(j = 0; j< i; ++j) {
				pthread_cancel(threads[j]);
				pthread_join(threads[j], &res);
			}
			log_msg("ERROR CREATING A THREAD IN NAN\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}
	}

	sprintf(msg, "A pool of %d threads has been created\n", args->start);
	if(log_msg(msg, args->output) < 0) {
		log_msg("ERROR LOGGING IN NAN\n", args->output);
		kill(getpid(), SIGINT);
		pthread_exit(0);
	}


	while(1) {
		if(sem_wait(args->pending_sem) < 0) {
			log_msg("ERROR WAITING SEMAPHORE IN NAN\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}

		if(*(args->term_flag) == 1) {
			log_msg("Termination signal received, waiting for ongoing threads to complete.\n", args->output);
			for(i = 0; i < current_threads; ++i) {
				sem_post(&req_sem);
			}
			for(i = 0; i < current_threads; ++i) {
				pthread_join(threads[i], &res);
			}
			log_msg("All Threads Completed\n", args->output);
			free_cache(cache);
			free_req_queue(reqs);
			pthread_exit(0);
		}

		// get the request fd from the server
		if(pthread_mutex_lock(args->pending_mutex) < 0) {
			log_msg("ERROR LOCKING MUTEX 1 IN NAN\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}
		current_client = get_queue_int(args->pending);
		if(pthread_mutex_unlock(args->pending_mutex) < 0) {
			log_msg("ERROR UNLOCKING MUTEX 1 IN NAN\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}

		// Dynamic increase
		if(pthread_mutex_lock(&status_mutex) < 0) {
			log_msg("ERROR LOCKING MUTEX 2 IN NAN\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}
		j = 0;
		for(i = 0; i < current_threads; ++i) {
			if(thread_status[i] == 1) {
				j++;
			}
		}
		sprintf(msg, "A connection has been delegated to one of the threads, system load %0.02lf%%\n", (double)((double)(j+1)/(double)current_threads)*100.0);
		log_msg(msg, args->output);

		if((double)((double)(j+1)/(double)current_threads) > 0.75 && current_threads < args->max) {
			i = 0.25 * current_threads;
			if(i + current_threads > args->max) {
				i = args->max - current_threads;
			}
			for(k = 0; k < i; ++k) {
				if(pthread_create(&(threads[k+current_threads]), NULL, unroll_thread, (void *)&thread_args[k+current_threads]) != 0) {
					for(j = 0; j< k+current_threads; ++j) {
						pthread_cancel(threads[j]);
						pthread_join(threads[j], &res);
					}
					log_msg("ERROR CREATING A THREAD IN NAN\n", args->output);
					kill(getpid(), SIGINT);
					pthread_exit(0);
				}
			}

			current_threads += i;
			sprintf(msg, "System load 75%% or over, pool extended to %d threads\n", current_threads);
			log_msg(msg, args->output);
		}
		if(pthread_mutex_unlock(&status_mutex) < 0) {
			log_msg("ERROR UNLOCKING MUTEX 2 IN NAN\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}

		if(recv(current_client, &current_request, sizeof(request), MSG_WAITALL) < 0) {
			log_msg("ERROR REVIVING PACKET\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}

		if(pthread_mutex_lock(&req_mutex) < 0) {
			log_msg("ERROR LOCKING MUTEX 3 IN NAN\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}
		add_req_queue(&reqs, current_request, current_client);
		if(pthread_mutex_unlock(&req_mutex) < 0) {
			log_msg("ERROR UNLOCKING MUTEX 3 IN NAN\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}

		if(sem_post(&req_sem) < 0) {
			log_msg("ERROR POSTING SEMPHORE IN NAN\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}
	}
}


void *unroll_thread(void *arguments) {
	threads_t *args = (threads_t*) arguments; // not to cast at every line
	req_queue current_request;
	char *path;
	char msg[128];
	int i, len;

	while(1) {
		sprintf(msg, "Thread #%d: waiting for a connection\n", args->id);
		log_msg(msg, args->output);

		if(sem_wait(args->req_sem) < 0) {
			log_msg("ERROR WAITING SEMPHORE IN THREAD\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}


		if(*(args->term_flag) == 1) {
			pthread_exit(0);
		}

		if(pthread_mutex_lock(args->status_mutex) < 0) {
			log_msg("ERROR LOCKING MUTEX 1 IN THREAD\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}
		args->thread_status[args->id] = 1;
		if(pthread_mutex_unlock(args->status_mutex) < 0) {
			log_msg("ERROR UNLOCKING MUTEX 1 IN THREAD\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}

		if(pthread_mutex_lock(args->req_mutex) < 0) {
			log_msg("ERROR LOCKING MUTEX 2 IN THREAD\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}
		current_request = get_req_queue(args->reqs);
		if(pthread_mutex_unlock(args->req_mutex) < 0) {
			log_msg("ERROR UNLOCKING MUTEX 2 IN THREAD\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}

		sprintf(msg, "Thread #%d: searching database for a path from node %d to node %d\n", args->id, current_request.req.source, current_request.req.dest);
		log_msg(msg, args->output);

		// readers
		if(args->priority == 0) {
			if(pthread_mutex_lock(args->read_mutex) < 0) {
				log_msg("ERROR LOCKING MUTEX 3 IN THREAD\n", args->output);
				kill(getpid(), SIGINT);
				pthread_exit(0);
			}
			*(args->read_count) += 1;
			if(*(args->read_count) == 1) {
				if(pthread_mutex_lock(args->cache_mutex) < 0) {
					log_msg("ERROR LOCKING MUTEX 4 IN THREAD\n", args->output);
					kill(getpid(), SIGINT);
					pthread_exit(0);
				}
			}
			if(pthread_mutex_unlock(args->read_mutex) < 0) {
				log_msg("ERROR UNLOCKING MUTEX 3 IN THREAD\n", args->output);
				kill(getpid(), SIGINT);
				pthread_exit(0);
			}

			path = is_cached(* (args->cache), current_request.req.source, current_request.req.dest);
			
			if(pthread_mutex_lock(args->read_mutex) < 0) {
				log_msg("ERROR LOCKING MUTEX 5 IN THREAD\n", args->output);
				kill(getpid(), SIGINT);
				pthread_exit(0);
			}
			*(args->read_count) -= 1;
			if(*(args->read_count) == 0) {
				if(pthread_mutex_unlock(args->cache_mutex) < 0) {
					log_msg("ERROR UNLOCKING MUTEX 4 IN THREAD\n", args->output);
					kill(getpid(), SIGINT);
					pthread_exit(0);
				}
			}
			if(pthread_mutex_unlock(args->read_mutex) < 0) {
				log_msg("ERROR UNLOCKING MUTEX 5 IN THREAD\n", args->output);
				kill(getpid(), SIGINT);
				pthread_exit(0);
			}

		} else if (args->priority == 1) {
			if(pthread_mutex_lock(args->read_entry) < 0) {
				log_msg("ERROR LOCKING MUTEX 6 IN THREAD\n", args->output);
				kill(getpid(), SIGINT);
				pthread_exit(0);
			}
			if(pthread_mutex_lock(args->read_mutex) < 0) {
				log_msg("ERROR LOCKING MUTEX 7 IN THREAD\n", args->output);
				kill(getpid(), SIGINT);
				pthread_exit(0);
			}
			*(args->read_count) += 1;
			if(*(args->read_count) == 1) {
				if(pthread_mutex_lock(args->cache_mutex) < 0) {
					log_msg("ERROR LOCKING MUTEX 8 IN THREAD\n", args->output);
					kill(getpid(), SIGINT);
					pthread_exit(0);
				}
			}
			if(pthread_mutex_unlock(args->read_mutex) < 0) {
				log_msg("ERROR UNLOCKING MUTEX 6 IN THREAD\n", args->output);
				kill(getpid(), SIGINT);
				pthread_exit(0);
			}
			if(pthread_mutex_unlock(args->read_entry) < 0) {
				log_msg("ERROR UNLOCKING MUTEX 7 IN THREAD\n", args->output);
				kill(getpid(), SIGINT);
				pthread_exit(0);
			}

			path = is_cached(*(args->cache), current_request.req.source, current_request.req.dest);
			
			if(pthread_mutex_lock(args->read_mutex) < 0) {
				log_msg("ERROR LOCKING MUTEX 9 IN THREAD\n", args->output);
				kill(getpid(), SIGINT);
				pthread_exit(0);
			}
			*(args->read_count) -= 1;
			if(*(args->read_count) == 0) {
				if(pthread_mutex_unlock(args->cache_mutex) < 0) {
				log_msg("ERROR UNLOCKING MUTEX 8 IN THREAD\n", args->output);
				kill(getpid(), SIGINT);
				pthread_exit(0);
			}
			}
			if(pthread_mutex_unlock(args->read_mutex) < 0) {
				log_msg("ERROR UNLOCKING MUTEX 9 IN THREAD\n", args->output);
				kill(getpid(), SIGINT);
				pthread_exit(0);
			}

		} else { // we checked for this value in the beggining, so what remains is 2
			if(pthread_mutex_lock(args->cache_mutex) < 0) {
				log_msg("ERROR LOCKING MUTEX 10 IN THREAD\n", args->output);
				kill(getpid(), SIGINT);
				pthread_exit(0);
			}
			path = is_cached(*(args->cache), current_request.req.source, current_request.req.dest);
			if(pthread_mutex_unlock(args->cache_mutex) < 0) {
				log_msg("ERROR UNLOCKING MUTEX 10 IN THREAD\n", args->output);
				kill(getpid(), SIGINT);
				pthread_exit(0);
			}
		}

		
		if(path == NULL) { // writers

			sprintf(msg, "Thread #%d: no path in database, calculating %d->%d\n", args->id, current_request.req.source, current_request.req.dest);
			log_msg(msg, args->output);

			path = BFS(*(args->graph), current_request.req.source, current_request.req.dest);

			if(args->priority == 0) {
				if(pthread_mutex_lock(args->cache_mutex) < 0) {
					log_msg("ERROR LOCKING MUTEX 11 IN THREAD\n", args->output);
					kill(getpid(), SIGINT);
					pthread_exit(0);
				}
				add_dest(*(args->cache), current_request.req.source, current_request.req.dest, path);
				if(pthread_mutex_unlock(args->cache_mutex) < 0) {
					log_msg("ERROR UNLOCKING MUTEX 11 IN THREAD\n", args->output);
					kill(getpid(), SIGINT);
					pthread_exit(0);
				}

			} else if (args->priority == 1) {
				if(pthread_mutex_lock(args->write_mutex) < 0) {
					log_msg("ERROR LOCKING MUTEX 12 IN THREAD\n", args->output);
					kill(getpid(), SIGINT);
					pthread_exit(0);
				}
				*(args->write_count) += 1;
				if(*(args->write_count) == 1) {
					if(pthread_mutex_lock(args->read_entry) < 0) {
						log_msg("ERROR LOCKING MUTEX 13 IN THREAD\n", args->output);
						kill(getpid(), SIGINT);
						pthread_exit(0);
					}
				}
				if(pthread_mutex_unlock(args->write_mutex) < 0) {
					log_msg("ERROR UNLOCKING MUTEX 12 IN THREAD\n", args->output);
					kill(getpid(), SIGINT);
					pthread_exit(0);
				}

				if(pthread_mutex_lock(args->cache_mutex) < 0) {
					log_msg("ERROR LOCKING MUTEX 14 IN THREAD\n", args->output);
					kill(getpid(), SIGINT);
					pthread_exit(0);
				}
				add_dest(*(args->cache), current_request.req.source, current_request.req.dest, path);
				if(pthread_mutex_unlock(args->cache_mutex) < 0) {
					log_msg("ERROR UNLOCKING MUTEX 13 IN THREAD\n", args->output);
					kill(getpid(), SIGINT);
					pthread_exit(0);
				}

				if(pthread_mutex_lock(args->write_mutex) < 0) {
					log_msg("ERROR LOCKING MUTEX 15 IN THREAD\n", args->output);
					kill(getpid(), SIGINT);
					pthread_exit(0);
				}
				*(args->write_count) -= 1;
				if(*(args->write_count) == 0) {
					if(pthread_mutex_unlock(args->read_entry) < 0) {
						log_msg("ERROR UNLOCKING MUTEX 14 IN THREAD\n", args->output);
						kill(getpid(), SIGINT);
						pthread_exit(0);
					}
				}
				if(pthread_mutex_unlock(args->write_mutex) < 0) {
					log_msg("ERROR UNLOCKING MUTEX 15 IN THREAD\n", args->output);
					kill(getpid(), SIGINT);
					pthread_exit(0);
				}

			} else { // we checked for this value in the beggining, so what remains is 2
				if(pthread_mutex_lock(args->cache_mutex) < 0) {
					log_msg("ERROR LOCKING MUTEX 16 IN THREAD\n", args->output);
					kill(getpid(), SIGINT);
					pthread_exit(0);
				}
				add_dest(*(args->cache), current_request.req.source, current_request.req.dest, path);
				if(pthread_mutex_unlock(args->cache_mutex) < 0) {
					log_msg("ERROR UNLOCKING MUTEX 16 IN THREAD\n", args->output);
					kill(getpid(), SIGINT);
					pthread_exit(0);
				}
			}

			if(path[0] == '\0') {
				sprintf(msg, "Thread #%d: path not possible\n", args->id);
				log_msg(msg, args->output);
			} else {
				sprintf(msg, "Thread #%d: path calculated: %s\n", args->id, path);
				log_msg(msg, args->output);
			}
		} else {

			if(path[0] == '\0') {
				sprintf(msg, "Thread #%d: path not possible\n", args->id);
				log_msg(msg, args->output);
			} else {
				sprintf(msg, "Thread #%d: path found in database: %s\n", args->id, path);
				log_msg(msg, args->output);
			}
		}

		// sendthing the length of the path, then the path itself character by character
		len = (path == NULL) ? 0 : strlen(path);
		if(send(current_request.fd, &len, sizeof(int), 0) < 0) {
			log_msg("ERROR SENDING PACKETS 1 IN THREAD\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}

		for(i = 0; i < len; i++) {
			if(send(current_request.fd, path+i, sizeof(char), 0) < 0) {
				log_msg("ERROR SENDING PACKETS 2 IN THREAD\n", args->output);
				kill(getpid(), SIGINT);
				pthread_exit(0);
			}
		}

		if(close(current_request.fd) < 0) {
			log_msg("ERROR CLOSING SOCKET FD IN THREAD\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}

		if(pthread_mutex_lock(args->status_mutex) < 0) {
			log_msg("ERROR LOCKING MUTEX 17 IN THREAD\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}
		args->thread_status[args->id] = 0;
		if(pthread_mutex_unlock(args->status_mutex) < 0) {
			log_msg("ERROR UNLOCKING MUTEX 17 IN THREAD\n", args->output);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}
	}
}

