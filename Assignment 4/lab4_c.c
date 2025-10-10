#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
	int n, d;
	pid_t pid;
	int status;

	if(argc != 3){
		fprintf(stderr, "Usage: %s <n> <d>\n", argv[0]);
		fprintf(stderr, "n: length of sequence\n");
		fprintf(stderr, "d: common difference\n");
		exit(1);
	}

	n = atoi(argv[1]);
	d = atoi(argv[2]);
	
	if(n <= 0){
		fprintf(stderr, "n must be positive\n");
	}

	// Create child process
	pid = fork();

	if(pid < 0){
		perror("fork failed");
		exit(1);
	}

	if(pid == 0){
		//Child process: print n elements of the sequence
		for(int k=0; k<n; k++){
			printf("%d, ", k*d);
		}
		exit(0);
	}
	else{
		// Parent process: print additional two elements of the sequence
		// Wait for the child to exit
		wait(&status);
		printf("%d, %d\n", n*d, (n+1)*d);	
	}

	return 0;
}
