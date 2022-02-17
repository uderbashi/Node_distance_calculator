#include "daemon.h"
#include <stdio.h>

#ifndef FILE_LOCK_INCSTANCE
#define FILE_LOCK_INCSTANCE "/tmp/.ONE_INSTANCE_171044078"
#endif

int become_daemon() {
	int fd;
	int lockfd;
	struct flock lock, check;


	if((lockfd = open(FILE_LOCK_INCSTANCE, O_RDWR | O_CREAT, 0006)) < 0) {
		de_write(STDERR_FILENO, "ERROR OPENING CHECK FILE OR ANOTHER INSTANCE OF THE PROGRAM IS RUNNING IN DAEMON\n");
		return -5;
	}
	memset(&lock, 0, sizeof(struct flock));
	memset(&check, 0, sizeof(struct flock));
	if(fcntl(lockfd, F_GETLK, &check) < 0){
		printf("%d\n", errno);
	}
	if(check.l_type == F_WRLCK) {
		de_write(STDERR_FILENO, "ANOTHER INSTANCE OF THE PROGRAM IS RUNNING\n");
		return -1;
	}
	lock.l_type = F_WRLCK;
	if(fcntl(lockfd, F_SETLKW, &lock) < 0){
		printf("%d\n", errno);
	}



	// Becoming a background process
	switch(fork()) {
		case -1:
			de_write(STDERR_FILENO, "ERROR FORKING 1 WHILE BECOMING DAEMON\n");
			return -6;
		case 0:
			break;
		default:
			exit(EXIT_SUCCESS);
	}

	// Become leader of new session
	if(setsid() == -1){
		de_write(STDERR_FILENO, "ERROR CREATING A NEW SESSION WHILE BECOMING DAEMON\n");
		return -6;
	}

	// Ensure we are not session leader
	switch (fork()) {
		case -1:
			de_write(STDERR_FILENO, "ERROR FORKING 2 WHILE BECOMING DAEMON\n");
			return -6;
		case 0:
			break;
		default:
			exit(EXIT_SUCCESS);
	}

	// Clear file mode creation mask, doesn't error
	umask(000);

	// Switch to root directory to avoid abrupt termination in case of unmounting
	if(chdir("/") == -1){
		de_write(STDERR_FILENO, "ERROR CHANGING DIRECTORY WHILE BECOMING DAEMON\n");
		return -5;
	}


	// Reopen standard fd's to /dev/null
	// I have not inherited any other files to close, but I do have my input and log fds that I don't want to close yet
	if(close(STDIN_FILENO) == -1) {
		de_write(STDERR_FILENO, "ERROR CLOSING STDIN WHILE BECOMING DAEMON\n");
		return -5;
	}

	if((fd = open("/dev/null", O_RDWR)) != STDIN_FILENO) {
		de_write(STDERR_FILENO, "ERROR OPENING /DEV/NULL WHILE BECOMING DAEMON\n");
		return -5;
	}
	if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO){
		de_write(STDERR_FILENO, "ERROR DUPLICATING INTO STDOUT WHILE BECOMING DAEMON\n");
		return -5;
	}
	if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO){
		de_write(STDERR_FILENO, "ERROR DUPLICATING INTO STDERR WHILE BECOMING DAEMON\n");
		return -5;
	}
	
	return 0;
}


void release_lock() {
	int lockfd;
	struct flock lock;

	lockfd = open(FILE_LOCK_INCSTANCE, O_RDWR, 0666);
	memset(&lock, 0, sizeof(struct flock));
	lock.l_type = F_UNLCK;
	fcntl(lockfd, F_SETLKW, &lock);
	unlink(FILE_LOCK_INCSTANCE);
}