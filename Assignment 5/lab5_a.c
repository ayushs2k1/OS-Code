#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

#define SHM_NAME "/shared_buffer"
#define BUF_SZ 5
#define SEM_MUTEX "/sem_mutex"
#define SEM_EMPTY "/sem_empty"
#define SEM_FULL "/sem_full"

// Shared Memory structure
typedef struct{
	double buffer[BUF_SZ];
	int in;
	int out;
	int count;
	int total_produced;
} shared_data;


void producer (int n, double d, double init_value, shared_data *shm, sem_t *mutex, sem_t *empty, sem_t *full){
	int i;
	srand(time(NULL) ^ getpid());
	
	for(int i=0; i<n; i++){
		// Generate the element
                double element = init_value + i * d;
        
                // Random wait between 0 and 3 seconds
                int sleep_time = rand() % 3;
                sleep(sleep_time);
                
                // Wait for empty slot
                sem_wait(empty);
                
                // Enter critical section
                sem_wait(mutex);
                
                // Add element to buffer
                shm->buffer[shm->in] = element;
                shm->in = (shm->in + 1) % BUF_SZ;
                shm->count++;
                shm->total_produced++;
                
                // Exit critical section
                sem_post(mutex);
                
                // Signal that buffer has item
                sem_post(full);
	}
}	

void consumer (int n, shared_data *shm, sem_t *mutex, sem_t *empty, sem_t *full){
        int i;
                
        for (i = 0; i < n; i++) {

                // Wait for full slot
                sem_wait(full);
                    
                // Enter critical section
                sem_wait(mutex);
                    
                // Remove element from buffer
                double element = shm->buffer[shm->out];
                shm->out = (shm->out + 1) % BUF_SZ;
                shm->count--;
                    
                // Exit critical section
                sem_post(mutex);
                    
                // Signal that buffer has empty slot
                sem_post(empty);
                    
                // Print the element immediately
                printf("%.1f\n", element);
                fflush(stdout);
        }               
}

int main (int argc, char *argv[]){
	if(argc != 3){
		fprintf(stderr, "Usage: %s <n> <d>\n", argv[0]);
		fprintf(stderr, "n: Integer > 1 (Number of elements)\n");
		fprintf(stderr, "d: Double (common difference)\n");
		return 1;
	}

	int n = atoi(argv[1]);
	double d = atof(argv[2]);

	if(n <= 1){
		fprintf(stderr, "Error: n must be greater than 1\n");
		return 1;
	}

	// Calculating init_value
	// Name: Ayush Sharma
	// ASCII Value of A = 65
	// ASCII Value of S = 83
	// init_value = ASCII Value of A + 1/10 (ASCII Value of S) = 65 + 1/10 * 83 = 73.3
	double init_value = 65.0 + ((1.0/10.0) * 83.0);

	// Creating Shared Memory
	int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
	if(shm_fd == -1){
		perror("shm_open");
		return 1;
	}

	// Set size of shared memory
	if(ftruncate(shm_fd, sizeof(shared_data)) == -1){
		perror("ftruncate");
		shm_unlink(SHM_NAME);
		return 1;
	}

	// Mapping shared memory
	shared_data *shm = (shared_data *)mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

	if(shm == MAP_FAILED){
		perror("mmap");
		shm_unlink(SHM_NAME);
		return 1;
	}

	// Initializing shared memory
	shm -> in = 0;
	shm -> out = 0;
	shm -> count = 0;
	shm -> total_produced = 0;


	// Creating Semaphores
	sem_t *mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
	sem_t *empty = sem_open(SEM_EMPTY, O_CREAT, 0666, BUF_SZ);
	sem_t *full = sem_open(SEM_FULL, O_CREAT, 0666, 0);

	if(mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED){
		perror("sem_open");
		munmap(shm, sizeof(shared_data));
		shm_unlink(SHM_NAME);
		return 1;
	}

	// Create chile process
	pid_t pid = fork();

	if(pid < 0){
		perror("fork");
		munmap(shm, sizeof(shared_data));
		shm_unlink(SHM_NAME);
		sem_close(mutex);
		sem_close(empty);
		sem_close(full);
		sem_unlink(SEM_MUTEX);
		sem_unlink(SEM_EMPTY);
		sem_unlink(SEM_FULL);
		return 1;
	}

	if(pid == 0){
		// Child Process - Producer
		producer(n, d, init_value, shm, mutex, empty, full);

		// Cleanup
		munmap(shm, sizeof(shared_data));
		sem_close(mutex);
		sem_close(empty);
		sem_close(full);
		exit(0);
	}

	else{
		// Parent Process - Consumer
		consumer(n, shm, mutex, empty, full);

		// Wait for child to finish
		wait(NULL);

		// Cleanup
		munmap(shm, sizeof(shared_data));
		shm_unlink(SHM_NAME);

		sem_close(mutex);
		sem_close(empty);
		sem_close(full);
		
		shm_unlink(SEM_MUTEX);
		shm_unlink(SEM_EMPTY);
		shm_unlink(SEM_FULL);
	}
	return 0;
}
