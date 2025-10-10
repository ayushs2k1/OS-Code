#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int create_process_tree(){
	pid_t pid1, pid2, pid3;

	// Parent creates the first child C1
	pid1 = fork();
	if (pid1 < 0){
		perror("fork failed");
		exit(1);
	}
	if (pid1 == 0){
		// This is first child C1
		return 1;
	}

	// Parent creates the second child C2
	pid2 = fork();
	if (pid2 < 0){
		perror("fork failed");
		exit(1);
	}
	if (pid2 == 0){
		// This is second child C2 and it creates child C3
		pid3 = fork();
		if (pid3 < 0){
			perror("fork failed");
			exit(1);
		}
		if (pid3 == 0){
			// This is third child C3
			return 3;
		}
		
		// C2 returns
		return 2;
	}

	// Parent process returns
	return 0;
}

int main(){
	int result = create_process_tree();

	printf("Process PID: %d, Parent PID: %d, Role: ", getpid(), getppid());

	switch(result){
		case 0:
			printf("Parent\n");
			// Wait for both direct childern C1 and C2
			wait(NULL);
			wait(NULL);
			break;
		case 1:
			printf("First Child C1\n");
			break;
		case 2:
			printf("Second Child C2\n");
			// Wait for the direct child C3
			wait(NULL);
			break;
		case 3:
			printf("Third Child C3\n");
			break;
	}

	return 0;
}
