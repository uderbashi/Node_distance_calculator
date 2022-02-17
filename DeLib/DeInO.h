#ifndef DEINO_H
#define DEINO_H

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>


// takes a successfuly opened file descriptor, and writes the message into it
// returns -2 upon failure, otherwise returns the amount of bytes written
int de_write(int fd, char *message);

// takes a successfuly opned file descriptor, locks it and writes the message into it, then unlocks it.
// returns negative number upon failure, otherwise the amount of bytes written
int de_write_lock(int fd, char *message);

// takes a successfuly opened file descriptor, and writes the float number up to three decimal points into it
// returns -2 upon failure, otherwise returns the amount of bytes written
int de_write_float(int fd, double n);

// takes a successfuly opened file descriptor, and writes the integer into it
// returns -2 upon failure, otherwise returns the amount of bytes written
int de_write_int(int fd, int n);

// takes a successfuly opened file descriptor, and writes a new line into it
// returns -2 upon failure, otherwise returns the amount of bytes written
int de_wnl(int fd);

// takes a successfuly opened file descriptor, and reads b bytes from it into buffer
// returns -3 upon failure, otherwise returns the amount of bytes read
int de_read(int fd, unsigned int b, char *buffer);

// takes a successfuly opened file descriptor, and reads until a new line or EOF into buffer
// returns -3 upon failure, otherwise returns the amount of bytes read
int de_rnl(int fd, char *buffer);

// like de_rnl but it locks the file while it is read and then removes the read part from the file
int de_rnl_lock(int fd, char *buffer);

// takes the file at the current cursor and shifts it to the beginning
int shifter(int fd);

#endif