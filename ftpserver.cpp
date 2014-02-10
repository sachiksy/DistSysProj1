#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>		//needed for creating Internet addresses

#define BUFFER 1024

struct thread_data{
	int sid;
	//char *string;
};

char homeDir[BUFFER];

//Prints the message from the clients and writes the same message back to the client
void *Echo (void *threadargs){
	int wCheck;
	struct thread_data *data;
	data=(struct thread_data *) threadargs;
	int sid=data->sid;
	char str[BUFFER];
	char cwd[BUFFER];
	strcpy(cwd, homeDir);
	
	while(strcmp(str, "exit")!=0){
		memset(str, '\0', BUFFER);
		if (read(sid, str, BUFFER)<0){
			perror("read");
			exit(-4);
		}
		
		//Tokenize the client's request
		
		char *command=(char *) malloc(BUFFER);
		char *cargs=(char *) malloc(BUFFER);
		strcpy(command, str);
		command = strtok (command," ");
		if (command != NULL){
			cargs = strtok (NULL, "\n");
		}
		
		//Switch for the 7 main commands (not including exit). Else is echo
		if(strcmp(command, "get")==0){
			printf("Please implement 'get'\n");
		}
		else if(strcmp(command, "put")==0){
			printf("Please implement 'put'\n");
		}
		else if(strcmp(command, "delete")==0){
			printf("Please implement 'delete'\n");
		}
		else if( (strcmp(command, "ls")==0) && cargs==NULL ){
			DIR *dir;
			struct dirent *entry;
			char lsContents[BUFFER]="";
			if (getcwd(cwd, sizeof(cwd)) == NULL){
				perror("Couldn't get current working directory");
				strcpy(str, "Couldn't get current working directory\n");
			}
			else if ((dir = opendir(cwd)) == NULL){
				perror("Opening the directory");
				strcpy(str, "Failed to open directory object\n");
			}
			else {
				printf("contents of root:\n");
				while ((entry = readdir(dir)) != NULL){
					printf("  %s\n", entry->d_name);
					strcat(lsContents, entry->d_name);
					strcat(lsContents, "\n");
				}
				closedir(dir);
				strcpy(str, lsContents);
			}
		}
		else if(strcmp(command, "cd")==0){
			if (cargs==NULL){
				printf("cd missing arguments\n");
				strcpy(str, "cd error: must have 1 argument\n");
			}
			else if (chdir(cargs)!=0){
				perror("chdir() error");
				strcpy(str, "Failed to change directory\n");
			}
			else{
				strcpy(str, "Successfully changed the working directory!\n");
			}
		}
		else if(strcmp(command, "mkdir")==0 ){
			if (cargs==NULL){
				strcpy(str, "mkdir error: must have 1 argument\n");
			}
			else{
				if (mkdir(cargs, S_IRWXU|S_IRGRP|S_IXGRP) != 0){
					perror("mkdir() error");
					strcpy(str, "mkdir failed to execute\n");
				}
				else{
					strcpy(str, cargs);
					strcat(str, " directory successfully created\n");
				}
			}
			
		}
		else if( (strcmp(str, "pwd")==0) && cargs==NULL ){
			if (getcwd(str, sizeof(str)) == NULL){
				perror("pwd error");
				strcpy(str, "pwd failed\n");
			}
			else{
				printf("CWD is: %s\n", str);
			}
		}
		
		else{
			printf("Unrecognised command: %s\n", str);
		}
		
		//write back to the client
		wCheck=write(sid, str, BUFFER);
		if (wCheck<0){
			perror("write\n");
			exit(-7);
		}
	}
	return NULL;
}

int main(int argc, char *argv[]){

	//kills zombie children
	signal(SIGCHLD, SIG_IGN);

	struct addrinfo hints, *results;
	int sockid, client, reuse;
	int numThreads=2; //tk
	pthread_t threads[numThreads];//tk
	struct thread_data data_arr[numThreads];//tk
	
	//checks for command-line params
	if(argc!=2){
		printf("Usage: %s <port number>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;				//IPv4
	hints.ai_socktype = SOCK_STREAM;		//TCP
	hints.ai_flags = AI_PASSIVE;			//use current IP
	
	if (getaddrinfo(NULL, argv[1], &hints, &results) < 0) {
		perror("Cannot resolve the address\n");
		exit(EXIT_FAILURE);
	}
	
	//create socket file descriptor
	sockid = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
	
	//check for socket creation failure
	if (sockid < 0) {
		perror("Cannot open socket\n");
		exit(EXIT_FAILURE);
	}
	
	printf("got a socket number: %d\n", sockid);
	
	//port reuse
	reuse = 1;
	if (setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) < 0) {
		perror("Cannot set reuse option for socket\n");
		exit(EXIT_FAILURE);
	}
	
	//bind socket to given port
	if (bind(sockid, results->ai_addr, results->ai_addrlen) < 0) {
		perror("Cannot bind socket to given port\n");
		close(sockid);
		exit(EXIT_FAILURE);
	}
	
	//initialize home directory
	if (getcwd(homeDir, sizeof(homeDir)) == NULL){
		perror("Home Directory init error");
	}
	
	//listen (queue of 5) for incoming connections
	if (listen(sockid, 5) < 0) {
		perror("Cannot listen to socket\n");
		close(sockid);
		exit(EXIT_FAILURE);
	}
	
	//Accepts a client and calls the echo function
	int m; //tk
	//store details about the client who has connected
	struct sockaddr_in saddr;
	unsigned int addrlen = sizeof(saddr);
	while (1){
		//accept and create a separate socket for client(s)
		printf("trying to connect\n");
		if ((client=accept(sockid, (struct sockaddr *) &saddr, &addrlen)) < 0) {
			perror("Cannot open client socket\n");
			exit(EXIT_FAILURE);
		}
		printf("connected\n");
		
		data_arr[0].sid=client;
		m = pthread_create(&threads[0], NULL, Echo, (void *) &data_arr[0]);
		if (m){
			perror("Pthread");
			exit(-5);
		}
		//printf("Thread quit!\n");
	}
	pthread_exit(NULL); //tk
	exit(0);
}
