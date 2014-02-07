#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
using namespace std;

struct thread_data{
	int sid;
	//char *string;
};

char homeDir[1024];

//Prints the message from the clients and writes the same message back to the client
void *Echo (void *threadargs){
	int wCheck;
	struct thread_data *data;
	data=(struct thread_data *) threadargs;
	int sid=data->sid;
	char str[1024];
	string boo="";
	
	while(strcmp(str, "exit")!=0){
		memset(str, '\0', 1024);
		if (read(sid, str, 1024)<0){
			perror("read");
			exit(-4);
		}
		
		/*
		We need to implement a string parser to tokenize commands
		*/
		
		//Switch for the 7 main commands (not including exit). Else is echo
		if(strcmp(str, "get")==0){
			printf("Please implement 'get'\n");
		}
		else if(strcmp(str, "put")==0){
			printf("Please implement 'put'\n");
		}
		else if(strcmp(str, "delete")==0){
			printf("Please implement 'delete'\n");
		}
		else if(strcmp(str, "ls")==0){
			DIR *dir;
			struct dirent *entry;
			char cwd[1024];
			string lsContents;
			if (getcwd(cwd, sizeof(cwd)) == NULL){
				perror("Couldn't get current working directory");
			}
			
			if ((dir = opendir(cwd)) == NULL)
				perror("Opening the directory");
			else {
				puts("contents of root:");
				while ((entry = readdir(dir)) != NULL){
					printf("  %s\n", entry->d_name);
					lsContents.append(entry->d_name);
					lsContents.append("\n");
				}
				closedir(dir);
				strcpy(str, lsContents.c_str());
			}
		}
		else if(strcmp(str, "cd")==0){
			printf("Please implement 'cd'\n");
		}
		else if(strcmp(str, "mkdir")==0){
			printf("Please implement 'mkdir'\n");
		}
		else if(strcmp(str, "pwd")==0){
			if (getcwd(str, sizeof(str)) == NULL){
				perror("pwd error");
			}
			printf("CWD is: %s\n", str);
		}
		else{
			printf("Echo: %s\n", str);
		}
		
		//write back to the client
		wCheck=write(sid, str, 1024);
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

	int sockid, client;
	unsigned int addrlen = 0;
	char buf[1024];
	struct sockaddr_in saddr;
	int numThreads=2; //tk
	pthread_t threads[numThreads];//tk
	struct thread_data data_arr[numThreads];//tk
	
	//checks for command-line params
	if(argc!=2){
		printf("Usage: need a port number\n");
		exit(-1);
	}
	
	//opens socket
	sockid=socket(PF_INET, SOCK_STREAM, 0);
	if (sockid<0){
		perror("socket");
		exit(-2);
	}
	
	printf("got a socket number: %d\n", sockid);
	
	saddr.sin_family=AF_INET;
	saddr.sin_addr.s_addr=htons(INADDR_ANY);
	saddr.sin_port=htons(atoi(argv[1]));
	
	//binds socket
	if(bind(sockid, (struct sockaddr *) &saddr, sizeof(saddr))<0){
		perror("bind");
		exit(-3);
	}
	
	//initialize home directory
	if (getcwd(homeDir, sizeof(homeDir)) == NULL){
		perror("Home Directory init error");
	}
	
	listen(sockid, 0);
	
	//Accepts a client and calls the echo function
	int m; //tk
	while (1){
		//check if connection is accepted
		if((client=accept(sockid, (struct sockaddr *) &saddr, &addrlen))<=0){
			continue;
		}
		data_arr[0].sid=client;
		m = pthread_create(&threads[0], NULL, Echo, (void *) &data_arr[0]);
		if (m){
			perror("Pthread");
			exit(-5);
		}
	}
	pthread_exit(NULL); //tk
	exit(0);

}
