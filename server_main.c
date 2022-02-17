#include "server_main.h"

struct for_handler {
	int input;
	int output;
	hash_graph graph;
	pthread_t nan;
	nans_t args;
	int term_flag;
	queue_int *pending;
	pthread_mutex_t pending_mutex;
	sem_t pending_sem;
	int sockfd;
} global;

void server_main(char *cinput, char *coutput, int port, int start, int max, int priority) {
	sigset_t mask;
	int sockfd, newfd; 
	struct sockaddr_in serverAddr;

	server_prep(cinput, coutput, start, max, priority); //open files here and block int sig
	if(become_daemon() < 0) {
		free_queue_int(global.pending);
		free_graph(global.graph);
		close(global.output);
		exit(-12);
	}
	parse_input(cinput, coutput, port, start, max, priority); //parse input and close it
	if(pthread_create(&(global.nan), NULL, unleash_nan, (void *)&(global.args)) != 0){
		log_msg("ERROR CREATING A THREAD MASK\n", global.output);
		free_queue_int(global.pending);
		free_graph(global.graph);
		close(global.output);
		exit(-12);
	}
	
	if((sigemptyset(&mask) == -1) || (sigaddset(&mask, SIGINT) == -1)) {
		log_msg("ERROR INITLAISING SIGNAL MASK\n", global.output);
		cleanup(-7);
	}
	
	if(pthread_sigmask(SIG_UNBLOCK, &mask, NULL) == -1) {
		log_msg("ERROR UNBLOCKING SIG MASK\n", global.output);
		cleanup(-7);
	}

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		log_msg("ERROR CREATING SOCKET\n", global.output);
		cleanup(-13);
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1) {
		log_msg("ERROR BINDING SOCKET\n", global.output);
		cleanup(-13);
	}

	if(listen(sockfd, 25) == -1) {
		log_msg("ERROR LISTENING TO SOCKET\n", global.output);
		cleanup(-13);
	}

	while(1) {
		if((newfd = accept(sockfd, NULL, NULL)) == -1) {
			log_msg("ERROR ACCEPTING FROM SOCKET\n", global.output);
			cleanup(-13);
		}

		if(pthread_mutex_lock(&(global.pending_mutex)) != 0){
			log_msg("ERROR LOCKING MUTEX IN NAN\n", global.output);
			cleanup(-11);
		}
		add_queue_int(&(global.pending), newfd);
		if(pthread_mutex_unlock(&(global.pending_mutex)) != 0){
			log_msg("ERROR UBLOCKING MUTEX IN NAN\n", global.output);
			cleanup(-11);
		}

		if(sem_post(&(global.pending_sem)) != 0){
			log_msg("ERROR POSTING A SEMAPHORE IN NAN\n", global.output);
			cleanup(-11);
		}
	}
}

void server_prep(char *cinput, char *coutput, int start, int max, int priority) {
	sigset_t mask;
	struct sigaction term;
	memset(&term, 0, sizeof(term));
	term.sa_handler = &cleanup;

	global.term_flag = 0;
	global.pending = NULL;
	pthread_mutex_init(&(global.pending_mutex), NULL);
	sem_init(&(global.pending_sem), 0, 0);
	

	if((sigemptyset(&mask) == -1) || (sigaddset(&mask, SIGINT) == -1)) {
		de_write(STDERR_FILENO, "ERROR INITLAISING SIGNAL MASK\n");
		exit(-7);
	}
	
	if(pthread_sigmask(SIG_BLOCK, &mask, NULL) == -1) {
		de_write(STDERR_FILENO, "ERROR BLOCKING SIG MASK\n");
		exit(-7);
	}

	if((sigaction(SIGINT, &term, NULL)) == -1) {
		de_write(STDERR_FILENO, "ERROR SETTING SIGINT RULE\n");
		exit(-7);
	}

	if((global.input = open(cinput, O_RDONLY)) < 0) {
		de_write(STDERR_FILENO, "ERROR OPENING THE INPUT FILE\n");
		exit(-5);
	}

	if((global.output = open(coutput, O_WRONLY | O_CREAT | O_EXCL, 0666)) < 0) {
		de_write(STDERR_FILENO, "ERROR OPENING OR DUPLICATE OUTPUT FILE\n");
		close(global.input);
		exit(-5);
	}

	global.args.term_flag = &(global.term_flag);
	global.args.output = global.output;
	global.args.start = start;
	global.args.max = max;
	global.args.priority = priority;
	global.args.graph = &global.graph;
	global.args.pending = &global.pending;
	global.args.pending_mutex = &(global.pending_mutex);
	global.args.pending_sem = &(global.pending_sem);
}


void parse_input(char *cinput, char *coutput, int port, int start, int max, int priority) {
	char line[128];
	int nodes, edges, i = 0;
	int source, dest;
	clock_t begin, finish;
	int bytes_read;

	if(log_msg("Executing with parameters:\n", global.output) < 0) {
		log_msg("ERROR WRITING TO THE OUTPUT FILE\n", global.output);
		close(global.input);
		close(global.output);
		exit(-2);
	}

	sprintf(line, "-i %s\n", cinput);
	if(log_msg(line, global.output) < 0) {
		log_msg("ERROR WRITING TO THE OUTPUT FILE\n", global.output);
		close(global.input);
		close(global.output);
		exit(-2);
	}

	sprintf(line, "-o %s\n", coutput);
	if(log_msg(line, global.output) < 0) {
		log_msg("ERROR WRITING TO THE OUTPUT FILE\n", global.output);
		close(global.input);
		close(global.output);
		exit(-2);
	}

	sprintf(line, "-p %d\n", port);
	if(log_msg(line, global.output) < 0) {
		log_msg("ERROR WRITING TO THE OUTPUT FILE\n", global.output);
		close(global.input);
		close(global.output);
		exit(-2);
	}

	sprintf(line, "-s %d\n", start);
	if(log_msg(line, global.output) < 0) {
		log_msg("ERROR WRITING TO THE OUTPUT FILE\n", global.output);
		close(global.input);
		close(global.output);
		exit(-2);
	}

	sprintf(line, "-x %d\n", max);
	if(log_msg(line, global.output) < 0) {
		log_msg("ERROR WRITING TO THE OUTPUT FILE\n", global.output);
		close(global.input);
		close(global.output);
		exit(-2);
	}

	sprintf(line, "-r %d (it sets by defualt to 1)\n", priority);
	if(log_msg(line, global.output) < 0) {
		log_msg("ERROR WRITING TO THE OUTPUT FILE\n", global.output);
		close(global.input);
		close(global.output);
		exit(-2);
	}


	if(log_msg("Loading graph...\n", global.output) < 0) {
		log_msg("ERROR WRITING TO THE OUTPUT FILE\n", global.output);
		close(global.input);
		close(global.output);
		exit(-2);
	}

	begin = clock();

	do {
		if(de_rnl(global.input, line) < 0) {
			log_msg("ERROR READING THE INPUT FILE\n", global.output);
			close(global.input);
			close(global.output);
			exit(-3);
		}
	} while(strncmp("# Nodes:", line, 8) != 0);

	sscanf(line, "%*[^0123456789]%d%*[^0123456789]%d", &nodes, &edges);

	if(de_rnl(global.input, line) < 0) {
		log_msg("ERROR READING THE INPUT FILE\n", global.output);
		close(global.input);
		close(global.output);
		exit(-3);
	}

	global.graph = create_graph(nodes);

	while((bytes_read = de_rnl(global.input, line)) > 0) {
		sscanf(line, "%d\t%d", &source, &dest);
		add_edge(global.graph, source, dest);
		++i;
	}

	if(i != edges) {
		log_msg("Something is wrong with the input file\n", global.output);
		free_queue_int(global.pending);
		free_graph(global.graph);
		close(global.output);
		close(global.input);
		exit(-1);
	}


	finish = clock();
	sprintf(line, "Graph loaded in %0.02lf seconds with %d nodes and %d edges\n", (double)(finish - begin)/CLOCKS_PER_SEC, nodes, edges);
	log_msg(line, global.output);
	if(close(global.input) < 0) {
		log_msg("WARNING CLOSING THE INPUT FILE\n", global.output);
	}
}


void cleanup(int signal) {
	void *res;
	global.term_flag = 1;
	sem_post(&global.pending_sem);
	pthread_join(global.nan, &res);
	free_queue_int(global.pending);
	free_graph(global.graph);
	log_msg("All threads have terminated, server shutting down.\n", global.output);
	close(global.output);
	release_lock();
	exit(signal);
}