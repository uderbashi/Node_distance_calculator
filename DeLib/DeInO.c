#include "DeInO.h"

int de_write(int fd, char *message) {
	int len = strlen(message);
	int bytes_written;
	int total = 0;

	while (total != len) {
		while((bytes_written = write(fd, message + total, len - total)) == -1 && (errno == EINTR));

		if (bytes_written < 0) { // if there's a write error other than getting inerrupted by a signal
			write(STDERR_FILENO, "ERROR WHILE WRITING\n", 20);
			return -2;
		}

		total += bytes_written;
	}

	return total;
}


int de_read(int fd, unsigned int bytes, char *buffer) {
	int bytes_read = -2; // initialise to a negative number so it wont initalise it to 0 and skip the loop, and not be equla to the number of bytes by mistake
	int total = 0;
	char char_read[bytes + 1];

	while(bytes_read != (int) bytes && bytes_read != 0) {
		while((bytes_read = read(fd, char_read, bytes - total)) == -1 && (errno == EINTR));

		if (bytes_read < 0) { // if there's a read error other than getting inerrupted by a signal
			write(STDERR_FILENO, "ERROR WHILE READING\n", 20);
			return -3;
		}

		char_read[bytes_read] = '\0';
		memcpy(buffer + total, char_read, bytes_read);
		total += bytes_read;
	}

	return total;
}

int de_rnl(int fd, char *buffer) {
	int bytes_read = -2; // initialise to a negative number so it wont initalise it to 0 and skip the loop, and not be equla to the number of bytes by mistake
	int total = 0;
	char char_read[2]; // char and a null

	while((bytes_read = de_read(fd, 1, char_read)) > 0 && char_read[0] != '\n') {
		memcpy(buffer + total, char_read, bytes_read);
		total++;
	}

	buffer[total] = '\0';

	if (bytes_read < 0) { // error propagated from de_read
		return bytes_read;
	}

	return total;
}

/*
** WON'T BE USING THESE FUNCTIONS HERE

int de_write_lock(int fd, char *message) {
	int bytes_written;
	struct flock check, set;

	// checking for existing locks
	memset(&check, 0, sizeof(check));
	do
	{
		if (fcntl(fd, F_GETLK, &check) == -1){ // if checking for the lock fails
			de_write(STDERR_FILENO, "ERROR WHILE CHECKING FOR LOCKS\n");
			return -4;
		}
		
	} while (check.l_type != F_UNLCK);

	// locking
	memset(&set, 0, sizeof(set));
	set.l_type = F_WRLCK;
	if (fcntl(fd, F_SETLKW, &set) == -1){
		de_write(STDERR_FILENO, "ERROR WHILE SETTING LOCK\n");	
		return -4;
	}

	if((bytes_written = de_write(fd, message)) < 0){
		set.l_type = F_UNLCK; // attempt to unlock the file gracefully
		fcntl(fd, F_SETLKW, &set);
		return bytes_written;
	}
	
	//unlocking
	set.l_type = F_UNLCK;
	if(fcntl(fd, F_SETLKW, &set) == -1){
		de_write(STDERR_FILENO, "ERROR WHILE UNLOCKING\n");	
		return -4;
	}

	return bytes_written;
}


int de_write_float(int fd, double n){
	int bytes_written;
	int total = 0;
	int whole = (int) n;
	int decimal = abs((n - whole) * 1000);
	whole = abs(whole);

	if(n < 0) {
		if((bytes_written = de_write(fd, "-")) < 0){
			return bytes_written;
		}
		total += bytes_written;
	}


	if((bytes_written = de_write_int(fd, whole)) < 0){
		return bytes_written;
	}
	total += bytes_written;

	if((bytes_written = de_write(fd, ".")) < 0){
		return bytes_written;
	}
	total += bytes_written;

	if (decimal < 100) {
		if (decimal < 10) {
			if((bytes_written = de_write_int(fd, 0)) < 0){
				return bytes_written;
			}
			total += bytes_written;
		}

		if((bytes_written = de_write_int(fd, 0)) < 0){
			return bytes_written;
		}
		total += bytes_written;
	}
	
	if((bytes_written = de_write_int(fd, decimal)) < 0){
		return bytes_written;
	}
	total += bytes_written;

	return total;
}


int de_write_int(int fd, int n) {
	int copy = n;
	int digits = 0;
	char *converted;
	int i = 0, current;
	int bytes_written;
	int neg_correction = 0;

	if (n == 0) {
		bytes_written = de_write(fd, "0");
		return bytes_written;
	}

	if (n < 0) {
		digits++;
		copy *= -1;
		neg_correction = -1;
	}

	while (copy != 0) {
		copy /= 10;
		digits++;
	}

	converted = malloc(digits + 1);

	if (n < 0) {
		converted[0] = '-';
		n *= -1;
	}

	for (; i < digits + neg_correction; ++i) {
		current = n % 10;
		n /= 10;
		converted[digits - 1 - i] = (char)(current + 48);
	}

	converted[digits] = '\0';

	bytes_written = de_write(fd, converted);

	free(converted);

	return bytes_written;
}


int de_wnl(int fd){
	return de_write(fd, "\n");
}


int de_rnl_lock(int fd, char *buffer) {
	int bytes_read, ret;
	struct flock check, set;
	off_t size;


	// checking for existing locks
	memset(&check, 0, sizeof(check));
	do
	{
		usleep(250000); // small wait before checking again
		if (fcntl(fd, F_GETLK, &check) == -1){ // if checking for the lock fails
			de_write(STDERR_FILENO, "ERROR WHILE CHECKING FOR LOCKS\n");
			return -4;
		}
		
	} while (check.l_type != F_UNLCK);

	// locking
	memset(&set, 0, sizeof(set));
	set.l_type = F_WRLCK;
	if (fcntl(fd, F_SETLKW, &set) == -1){
		de_write(STDERR_FILENO, "ERROR WHILE SETTING LOCK\n");	
		return -4;
	}

	if((bytes_read = de_rnl(fd, buffer)) < 0){
		set.l_type = F_UNLCK; // attempt to unlock the file gracefully
		fcntl(fd, F_SETLKW, &set);
		return bytes_read;
	}


	// truncating
	if(bytes_read > 0) {
		if((ret = shifter(fd)) < 0) {
			de_write(STDERR_FILENO, "ERROR WHILE SHIFTING\n");	
			return ret;
		}

		if((size = lseek(fd, 0, SEEK_END)) < 0){
			de_write(STDERR_FILENO, "ERROR WHILE SEEKING\n");	
			return -8;
		}

		if(ftruncate(fd, size - bytes_read - 1) < 0) {
			de_write(STDERR_FILENO, "ERROR WHILE TRUNCATING FILE\n");	
			return -5;
		}

		if(lseek(fd, 0, SEEK_SET)< 0){
			de_write(STDERR_FILENO, "ERROR WHILE SEEKING\n");	
			return -8;
		}
	}

	//unlocking
	set.l_type = F_UNLCK;
	if(fcntl(fd, F_SETLKW, &set) == -1){
		de_write(STDERR_FILENO, "ERROR WHILE UNLOCKING\n");	
		return -4;
	}

	return bytes_read;
}


int shifter(int fd) {
	off_t current, end, shift;
	char char_read[2];
	int bytes_read;

	if((current = lseek(fd, 0, SEEK_CUR)) < 0) {
		de_write(STDERR_FILENO, "ERROR WHILE SEEKING\n");	
		return -8;
	}

	if((end = lseek(fd, 0, SEEK_END)) < 0) {
		de_write(STDERR_FILENO, "ERROR WHILE SEEKING\n");	
		return -8;
	}

	shift = current;
	current--;

	while((++current) <= end) {
		if(lseek(fd, current, SEEK_SET) < 0) {
			de_write(STDERR_FILENO, "ERROR WHILE SEEKING\n");	
			return -8;
		}

		if ((bytes_read = de_read(fd, 1, char_read)) < 0){
			return bytes_read; // propagate error
		}

		if(lseek(fd, current - shift, SEEK_SET) < 0) {
			de_write(STDERR_FILENO, "ERROR WHILE SEEKING\n");	
			return -8;
		}

		if ((bytes_read = de_write(fd, char_read)) < 0){
			return bytes_read; // propagate error
		}
	}
}

*/