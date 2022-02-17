#ifndef CLIENT_MAIN_H
#define CLIENT_MAIN_H

#include "DeLib/DeInO.h"
#include "data_structs/req_queue.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h> 

void client_main(char *address, int port, int source, int dest);
int log_msg(char* msg, int fd);
struct timeval diff_timeval(struct timeval t2, struct timeval t1);

#endif