#include "DeLib/DeInO.h"
#include "client_main.h"


#ifndef TEST_STATUS // set to 1 if the test is going to occur, otherwise 0
#define TEST_STATUS 0
#endif

void init_args(char **address, int *port, int *source, int *dest, int argc, char *argv[]);
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
*/
int main(int argc, char *argv[]) {
	//char message[128];
	char *address; // inout file name
	int port;
	int source;
	int dest;

	if(TEST_STATUS) { // when true it only runs a test function
		test();
		exit(EXIT_SUCCESS);
	}
	
	init_args(&address, &port, &source, &dest, argc, argv);

	client_main(address, port, source, dest);
	
	exit(EXIT_SUCCESS);
}


void init_args(char **address, int *port, int *source, int *dest, int argc, char *argv[]) {
	
	int PROGRAM_ARGS = 4;
	int i;
	int opt;
	int args[PROGRAM_ARGS]; // histogram of used arguments
	int error_flag = 0;

	for(i = 0; i < PROGRAM_ARGS; ++i) {
		args[i] = 0;
	}

	if(argc != PROGRAM_ARGS * 2 + 1){ // if argument number is insufficient
		error_flag = 1;
	} 

	while((opt = getopt(argc, argv, "a:p:s:d:")) != -1 && error_flag != 1) {
		switch (opt) {
		case 'a':
			args[0]++;
			*address = optarg;
			break;
		case 'p':
			args[1]++;
			*port = atoi(optarg);
			break;
		case 's':
			args[2]++;
			*source = atoi(optarg);
			break;
		case 'd':
			args[3]++;
			*dest = atoi(optarg);
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

	if(*port < 1 || *source < 0 || *dest < 0) { // invalid values for the variables
		error_flag = 1;
	}

	if(error_flag == 1) {
		args_instructions();
		exit(-1);
	}
}


void args_instructions() {
	char *message = "Wrong usage of the program. The right usage is:\n\033[1m./client -a address -p PORT  -s source -d destination \033[0m\nwhere \033[1minputPath\033[0m is the IP address of the machine running the server, and \033[1mPORT\033[0m, \033[1msource\033[0m and \033[1mdestination\033[0m are positive integers\n";
	int exit_stat;
	
	if((exit_stat = de_write(STDOUT_FILENO, message)) != 0) {
		exit(exit_stat);
	}
}


void test(){

}