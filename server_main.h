#ifndef SERVER_MAIN_H
#define SERVER_MAIN_H

#include "DeLib/DeInO.h"
#include "data_structs/hashgraph.h"
#include "data_structs/queue_int.h"
#include "daemon.h"
#include "nans_threads.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h> 

void server_main(char *cinput, char *coutput, int port, int start, int max, int priority);
void server_prep(char *cinput, char *coutput, int start, int max, int priority);
void parse_input();
void cleanup(int signal);


#endif