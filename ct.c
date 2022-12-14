#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#define MAX_CLIENTS 10
#define BUFFER_SZ 2048
#define NAME_LEN 32
//Client file 
// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[NAME_LEN];

void str_overwrite_stdout() {
  printf("%s", "> ");
  fflush(stdout);
}

void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}
void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void send_msg_handler() {
  char message[BUFFER_SZ] = {};
	char buffer[BUFFER_SZ + NAME_LEN] = {};

  while(1) {
  	str_overwrite_stdout();
    fgets(message, BUFFER_SZ, stdin);
    str_trim_lf(buffer, BUFFER_SZ);

    if (strcmp(message, "exit") == 0) {
			break;
    } else {
       sprintf(buffer, "%s %s", name,message);
      send(sockfd,message, strlen(message), 0);
    }
    
		bzero(buffer, BUFFER_SZ);
    bzero(message, BUFFER_SZ);
  }
  catch_ctrl_c_and_exit(2);
}

void recv_msg_handler() {
	char message[BUFFER_SZ] = {};
  while (1) {
		int receive = recv(sockfd, message, BUFFER_SZ, 0);
    if (receive > 0) {
     printf("%s", message);
      str_overwrite_stdout();
    } else if (receive == 0) {
			break;
    } 
		bzero(message, BUFFER_SZ);
  }
}

int main(int argc, char **argv){
	if(argc != 2){
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}
	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);

	signal(SIGINT, catch_ctrl_c_and_exit);

	printf("Please enter your name: ");
  fgets(name, NAME_LEN, stdin);
  str_trim_lf(name, strlen(name));


	if (strlen(name) > NAME_LEN -1 || strlen(name) < 2){
		printf("Name must be less than 30 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}
	struct sockaddr_in server_addr;

	/* Socket settings */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);


  // Connect to Server
  int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}
// Send name
	send(sockfd, name, NAME_LEN, 0);

	printf("=== WELCOME TO THE CHATROOM ===\n");

	pthread_t send_msg_thread;
  if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
    return EXIT_FAILURE;
	}

	pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}
while (1){
		if(flag){
			printf("\nBye\n");
			break;
    }
	}
	
	close(sockfd);

	return EXIT_SUCCESS;
	}
