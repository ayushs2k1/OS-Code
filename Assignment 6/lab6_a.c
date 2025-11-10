#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>

#define PORT 8080
#define BUFFER_SIZE 256


int main(){
	pid_t pid;
	int sockfd, newsockfd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len;

	pid = fork();

	if(pid < 0){
		perror("Fork Failed!");
		exit(1);
	}

	if(pid > 0){
		// Parent Process - Server (Consumer)
		printf("[Server] Parent process started (PID: %d)\n", getpid());

		// Insert random wait (0 to 6999 ms) before starting to listen
		srand(time(NULL));
		int wait_time = rand() % 7000;
		printf("[Server] Waiting %d ms before starting...\n", wait_time);
		usleep(wait_time * 1000);

		// Creating socket
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if(sockfd < 0){
			perror("Error opening socket!");
			exit(1);
		}

		// Allow socket reuse to avoid "Address already in use" errors
		int opt = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		// Initialize server address structure
		memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
    	server_addr.sin_addr.s_addr = INADDR_ANY;
    	server_addr.sin_port = htons(PORT);

		// Bind socket to port
        if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
            perror("Error on binding!");
            close(sockfd);
        	exit(1);
    	}

		// Listen for connections (backlog of 5)
    	listen(sockfd, 5);
    	printf("[Server] Listening on port %d...\n", PORT);

		// Accept Connection from client
		client_len = sizeof(client_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (newsockfd < 0){
        	perror("Error on accepting connection from client!");
            close(sockfd);
          	exit(1);
        }

		printf("[Server] Connection accepted from client\n");

		// Read messages from client
        char buffer[BUFFER_SIZE];
        int n;
        int message_count = 0;

		while (1){
           	memset(buffer, 0, BUFFER_SIZE);
            n = read(newsockfd, buffer, BUFFER_SIZE - 1);
            
           	if (n < 0){
        		perror("Error reading from socket!");
        		break;
            } 
            else if (n == 0){
                printf("[Server] Client closed connection\n");
                break;
            }

            printf("[Server] Received message %d\n", ++message_count);
            
            // Check for termination message
            if (strncmp(buffer, "END", 3) == 0){
                printf("[Server] Received END signal, shutting down\n");
            	break;
        	}
            
            // Send acknowledgment back to client
            char ack[BUFFER_SIZE];
    		snprintf(ack, BUFFER_SIZE, "ACK %d", message_count);
        	write(newsockfd, ack, strlen(ack));
		}
       		
        // Close sockets
        close(newsockfd);
        close(sockfd);
        
        // Wait for child process to finish
        wait(NULL);
        printf("[Server] Server terminated\n");
    }
	else{
		// Child process - Client (Producer)
		printf("[Client] Child process started (PID: %d)\n", getpid());

		// Create socket
    	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    	if (sockfd < 0){
            perror("Error opening socket!");
    		exit(1);
        }

		// Initialize server address
    	memset(&server_addr, 0, sizeof(server_addr));
     	server_addr.sin_family = AF_INET;
       	server_addr.sin_port = htons(PORT);

        // Convert IPv4 address from text to binary
        if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0){
            perror("Invalid address!");
            close(sockfd);
        	exit(1);
        }

		// Repeatedly attempt to connect until successful
        printf("[Client] Attempting to connect to server...\n");
        int connected = 0;
        int attempts = 0;

    	while (!connected){
        	attempts++;
            if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
                if (errno == ECONNREFUSED){
                    // Server not ready, wait and retry
                    printf("[Client] Connection refused, retrying... (attempt %d)\n", attempts);
                    usleep(100000); // Wait 100 ms
                } 
			    else{
                    perror("Connection error!");
        			close(sockfd);
                	exit(1);
                }
            } 
			else{
        		connected = 1;
                printf("[Client] Connected to server after %d attempts\n", attempts);
        	}
        }

    	// Send messages to server
        char buffer[BUFFER_SIZE];
        for (int i = 1; i <= 5; i++){
            snprintf(buffer, BUFFER_SIZE, "Message %d from producer", i);
    		printf("[Client] Sending: %s\n", buffer);
            write(sockfd, buffer, strlen(buffer));

            // Read acknowledgment
            memset(buffer, 0, BUFFER_SIZE);
            read(sockfd, buffer, BUFFER_SIZE - 1);
            printf("[Client] Received: %s\n", buffer);

            usleep(500000); // Wait 500 ms between messages
        }

        // Send termination message
        strcpy(buffer, "END");
        printf("[Client] Sending: %s\n", buffer);
        write(sockfd, buffer, strlen(buffer));

        // Close socket
        close(sockfd);
        printf("[Client] Client terminated\n");
        exit(0);
    }
    return 0;
}