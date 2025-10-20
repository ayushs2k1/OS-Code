#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

#define BUF_SZ 5

void producer(int pipefd, int n, double d, double init_value){
	int i;
	srand(time(NULL) ^ getpid());

	for(int i=0; i<n; i++){
		// Generate Element
		double element = init_value + i * d;

		// Random wait between 0 and 3 seconds
		int sleep_time = rand() % 3;
		sleep(sleep_time);

		// Write element to the pipe
		if(write(pipefd, &element, sizeof(double)) == -1){
			perror("write");
			exit(1);
		}

	}

	// Close write end when done
	close(pipefd);
}

void consumer(int pipefd, int n){
	int i;
	double element;

	for(int i=0; i<n; i++){
		// Read element from the pipe
		ssize_t bytes_read = read(pipefd, &element, sizeof(double));

		if(bytes_read == -1){
			perror("read");
			exit(1);
		}

		if(bytes_read == 0){
			fprintf(stderr, "Error: Pipe closed prematurely\n");
			exit(1);
		}

		// Print the element immediately
		printf("%.1f\n", element);
		fflush(stdout);
	}

	// Close read end when done
	close(pipefd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <n> <d>\n", argv[0]);
        fprintf(stderr, "  n: integer > 1 (number of elements)\n");
        fprintf(stderr, "  d: double (common difference)\n");
        return 1;
    }

    // Parse command line arguments
    int n = atoi(argv[1]);
    double d = atof(argv[2]);

    if (n <= 1) {
        fprintf(stderr, "Error: n must be greater than 1\n");
        return 1;
    }

    // Calculate init_value: 
    // ASCII Value of A = 65 
    // ASCII Value of S = 83
    // init_value = 65 + (1/10 * 83) = 65 + 8.3 = 73.3
    double init_value = 65.0 + ((1.0/10.0) * 83.0);

    // Create Pipe
    int pipefd[2];
    if(pipe(pipefd) == -1){
    	perror("pipe");
	return 1;
    }

    // Create Child Process
    pid_t pid = fork();

    if(pid < 0){
    	perror("fork");
	close(pipefd[0]);
	close(pipefd[1]);
	return 1;
    }

    if(pid == 0){
	    // Child Process - Producer
	    
	    // Close unused read end
	    close(pipefd[0]);

	    producer(pipefd[1], n, d, init_value);

	    exit(0);
    }

    else{
	    // Parent Process - Consumer

	    // Close unused write end
	    close(pipefd[1]);

	    consumer(pipefd[0], n);

	    // Wait for the child to finish
	    wait(NULL);
    }

    return 0;
}
