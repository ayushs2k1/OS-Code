#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>

#define NUM_THREADS 4
#define NUM_POINTS 1000000


// Shared variable for counting points inside circle
int count =0;

// Semaphore to protect critical section
sem_t semaphore;


// Worker Thread function
void *WorkerThread(void *arg){
	// Seed random number generator with unique value for each thread
	unsigned int rand_state = (unsigned int) time(NULL) + pthread_self();

	// Generate random points
	for(int i=0; i<NUM_POINTS; i++){
		// Generate random x and y coordinates between -1 and 1
		double x = (double)rand_r(&rand_state) / RAND_MAX * 2.0 - 1.0;
		double y = (double)rand_r(&rand_state) / RAND_MAX * 2.0 - 1.0;

		// Calculate radius
		double r = sqrt(x * x + y * y);

		// Check if point is inside the circle
		if(r <= 1.0){
			// Enter Critical Section
			sem_wait(&semaphore);
			count++;
			sem_post(&semaphore);
			// Exit Critical Section
		}
	}

	printf("Thread %lu completed. Generated %d points.\n", pthread_self(), NUM_POINTS);
	pthread_exit(NULL);
}

int main(){
	pthread_t threads[NUM_THREADS];
	int rc;

    printf("Monte Carlo Simulation to Estimate Pi\n");
	printf("======================================\n");
	printf("Number of threads: %d\n", NUM_THREADS);
	printf("Points per thread: %d\n", NUM_POINTS);
	printf("Total points: %d\n", NUM_THREADS * NUM_POINTS);
	printf("\n");


	// Initialize semaphore (binary semaphore with initial value as 1)
	if(sem_init(&semaphore, 0, 1) !=0){
		perror("Semaphore initialization failed!");
    	exit(1);
	}

	// Create worker threads
	printf("Creating %d worker threads...\n", NUM_THREADS);
	for(int i=0; i<NUM_THREADS; i++){
		rc = pthread_create(&threads[i], NULL, WorkerThread, NULL);
		if(rc){
			printf("Error: Unable to create thread %d, return code %d\n", i, rc);
			exit(1);
		}
	}

	// Wait for all threads to complete
	printf("\nWaiting for threads to complete...\n");
	for(int i=0; i<NUM_THREADS; i++){
		pthread_join(threads[i], NULL);
	}

	// Calculate and print the estimate value of pi
	int total_points = NUM_THREADS * NUM_POINTS;
	double pi_estimate = 4.0 * (double)count / (double)total_points;

	printf("\n======================================\n");
    printf("Results:\n");
    printf("Points inside circle: %d\n", count);
    printf("Total points: %d\n", total_points);
    printf("Estimated area of circle: %.6f\n", pi_estimate);
    printf("Actual value of pi: %.6f\n", M_PI);
    printf("Error: %.4f%%\n", fabs(pi_estimate - M_PI) / M_PI * 100);
    printf("======================================\n");

	// Destroy Semaphore
	sem_destroy(&semaphore);

	return 0;
}