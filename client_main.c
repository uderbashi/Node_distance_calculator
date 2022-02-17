#include "client_main.h"

void client_main(char *address, int port, int source, int dest) {
	int sockfd; 
	struct sockaddr_in serverAddr;
	request req;
	char *path;
	int len, i;
	char msg[128];
	struct timeval begin, finish, result;

	req.source = source;
	req.dest = dest;
	
	sprintf(msg, "Client (%d) connecting to %s:%d\n", getpid(), address, port);
	log_msg(msg, STDOUT_FILENO);

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		log_msg("ERROR OPENING SOCKET\n", STDOUT_FILENO);
		exit(-13);
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(address);

	if(connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1) {
		log_msg("ERROR CONNECTING TO A SOCKET SOCKET\n", STDOUT_FILENO);
		exit(-13);
	}

	sprintf(msg, "Client (%d) connected and requesting a path from node %d to %d\n", getpid(), source, dest);
	log_msg(msg, STDOUT_FILENO);

	gettimeofday(&begin, NULL);

	if(send(sockfd, &req, sizeof(request), 0) < 0) {
		log_msg("ERROR SENDING TO A SOCKET SOCKET\n", STDOUT_FILENO);
		exit(-13);
	}

	if(recv(sockfd, &len, sizeof(int), MSG_WAITALL) < 0) {
		log_msg("ERROR RECIVING FROM A SOCKET SOCKET\n", STDOUT_FILENO);
		exit(-13);
	}
	path = (char *)malloc((len + 1) * sizeof(char));

	for(i = 0; i < len; i++){
		if(recv(sockfd, path + i, sizeof(char), MSG_WAITALL) < 0) {
			log_msg("ERROR RECIVING FROM A SOCKET SOCKET\n", STDOUT_FILENO);
			exit(-13);
		}
	}
	path[i] = '\0';

	gettimeofday(&finish, NULL);
	result = diff_timeval(finish, begin);

	if(len == 0) {
		sprintf(msg, "Server’s response (%d): NO PATH, arrived in %ld.%ld seconds, shutting down\n", getpid(), result.tv_sec, result.tv_usec/10000);
		log_msg(msg, STDOUT_FILENO);
	} else {
		sprintf(msg, "Server’s response to (%d): %s, arrived in %ld.%ld seconds.\n", getpid(), path, result.tv_sec, result.tv_usec/10000);
		log_msg(msg, STDOUT_FILENO);
	}

	if(close(sockfd) < 0) {
		log_msg("ERROR CLOSING A SOCKET SOCKET\n", STDOUT_FILENO);
		exit(-13);
	}
	free(path);
}


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


struct timeval diff_timeval(struct timeval t2, struct timeval t1) {
    struct timeval result;

    result.tv_sec = t2.tv_sec - t1.tv_sec;
    result.tv_usec = t2.tv_usec - t1.tv_usec;

    while (result.tv_usec < 0) {
        result.tv_usec += 1000000;
        result.tv_sec--;
    }
    return result;
}