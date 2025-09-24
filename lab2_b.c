#include<unistd.h>
#include<sys/types.h>
#include<stdlib.h>
#include<time.h>
#include<fcntl.h>
#include<stdio.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
	// Check that only one argument (filename) was provided
	if(argc!=2){
		// Write usage message to STDERR
		write(STDERR_FILENO, "Usage: mycat <filename>\n", 24);
		exit(1);
	}

	// Get the PID of the running program
	pid_t pid=getpid();
        printf("Process ID: %d\n", pid);

	// Intialize random seed and generate random sleep time (1-5 seconds)
	srand(time(NULL));
	int sleep_time=(rand()%5)+1;

	// Sleep for random number of seconds
	sleep(sleep_time);

	// Open the file for reading only
	int fd=open(argv[1], O_RDONLY);
	
	// If open() fails
	if(fd<0){
		const char *msg="open: ";
		write(STDERR_FILENO, "open: No such file or directory\n", 32);
		exit(1);
	}

	// Keeps track of how many bytes were actually read by read()
	// It can be
	// >0 - Success (number of bytes read)
	// =0 - EOF
	// -1 - Error
	ssize_t bytes_read;

	// Temporary memory to hold file contents as they are read.
	char buffer[BUFFER_SIZE];

	while((bytes_read=read(fd, buffer, BUFFER_SIZE))>0){
		// Write the bytes we just read to STDOUT
		write(STDOUT_FILENO, buffer, (size_t)bytes_read);
	}

	// After done reading (or on error), close the file descriptor
	close(fd);

	return 0;
}
