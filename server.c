#include "DeLib/DeInO.h"
#include "server_main.h"
#include "data_structs/hashgraph.h"
#include "data_structs/bfs.h"
#include "nans_threads.h"
#include <time.h>
#include <stdio.h>


#ifndef TEST_STATUS // set to 1 if the test is going to occur, otherwise 0
#define TEST_STATUS 0
#endif

void init_args(char **cinput, char **coutput, int *port, int *start, int *max, int *priority, int argc, char *argv[]);
void args_instructions();
void test();


/*
** Exit Statuses:
** -1:  command line arguments issue
** -2:  writing to a file issue
** -3:  reading a file issue
** -4:  locking mechanism issue
** -5:  file issue
** -6:  fork issue
** -7:  signal issue
** -8:  seeking failure
** -9:  pipe issue
** -10: memory issue
** -11: semaphores issue
** -12: thread issue
** -13:	socket issue
*/
int main(int argc, char *argv[]) {
	//char message[128];
	char *cinput; // inout file name
	char *coutput; // output file name
	int port;
	int start;
	int max;
	int priority = 1;

	if(TEST_STATUS) { // when true it only runs a test function
		test();
		exit(EXIT_SUCCESS);
	}
	
	init_args(&cinput, &coutput, &port, &start, &max, &priority, argc, argv);

	server_main(cinput, coutput, port, start, max, priority);

	
	exit(EXIT_SUCCESS);
}


void init_args(char **cinput, char **coutput, int *port, int *start, int *max, int *priority, int argc, char *argv[]) {
	
	int PROGRAM_ARGS = 5;
	int i;
	int opt;
	int args[PROGRAM_ARGS]; // histogram of used arguments
	int error_flag = 0;

	for(i = 0; i < PROGRAM_ARGS; ++i) {
		args[i] = 0;
	}

	if(argc != PROGRAM_ARGS * 2 + 1 && argc != PROGRAM_ARGS * 2 + 3){ // if argument number is insufficient
		error_flag = 1;
	} 

	while((opt = getopt(argc, argv, "i:p:o:s:x:r:")) != -1 && error_flag != 1) {
		switch (opt) {
		case 'i':
			args[0]++;
			*cinput = optarg;
			break;
		case 'p':
			args[1]++;
			*port = atoi(optarg);
			break;
		case 'o':
			args[2]++;
			*coutput = optarg;
			break;
		case 's':
			args[3]++;
			*start = atoi(optarg);
			break;
		case 'x':
			args[4]++;
			*max = atoi(optarg);
			break;
		case 'r':
			*priority = atoi(optarg);
			break;
		case '?':
		default:
			error_flag = 1;
		}
	}

	for(i = 0; i < PROGRAM_ARGS && error_flag != 1; ++i) { // check that every entry in histogram is 1 ie. everything is used only once
		if (args[i] != 1) {
			error_flag = 1;
		}
	}

	if(*port < 1 || *start < 1 || *max < 1 || *start > *max) { // invalid values for the variables
		error_flag = 1;
	}

	if (*priority != 0 && *priority != 1 && *priority != 2) {
		error_flag = 1;
	}

	if(error_flag == 1) {
		args_instructions();
		exit(-1);
	}
}


void args_instructions() {
	char *message = "Wrong usage of the program. The right usage is:\n\033[1m./server -i inputPath -p PORT -o outputPath -s startingThreads -x maxThreads \033[0m\nwhere \033[1minputPath\033[0m is a string of a path leading to a file, \033[1moutputPath\033[0m is a string of a path leading to the desired location of a log file, and \033[1mPORT\033[0m, \033[1mstartingThreads\033[0m and \033[1mmaxThreads\033[0m are nonzero positive integers\n\033[1m-r flag\033[0m is an optional priority flag. Where the values 0, 1, and 2 represent readers, writers and equal priority respectively.\n";
	int exit_stat;
	
	if((exit_stat = de_write(STDOUT_FILENO, message)) != 0) {
		exit(exit_stat);
	}
}


void test(){
	// hash_graph graph = create_graph(8);
	// add_edge(graph, 1, 2);
	// add_edge(graph, 1, 4);
	// add_edge(graph, 4, 6);
	// add_edge(graph, 4, 7);
	// add_edge(graph, 4, 9);
	// add_edge(graph, 2, 9);
	// add_edge(graph, 2, 10);
	// add_edge(graph, 9, 4);
	// add_edge(graph, 7, 13);
	// add_edge(graph, 6, 15);
	// add_edge(graph, 10, 11);
	// add_edge(graph, 13, 10);
	// add_edge(graph, 13, 15);

	// char *a = BFS(graph, 1, 9);

	// hash_cache c = create_cache(10);
	// char *b = is_cached(c, 1, 9);

	// add_dest(c, 1, 9, a);
	// b = is_cached(c, 1, 9);
	// printf("%s\n",b );

	// free_graph(graph);
	// free_cache(c);

}