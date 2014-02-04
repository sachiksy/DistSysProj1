#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

struct thread_data{
	int sid;
	char *string;
};

//Prints the message from the clients and writes the same message back to the client
void *Echo (void *threadargs){ //tk (int sid, char *str)
	int wCheck;
	
	struct thread_data *data;
	data=(struct thread_data *) threadargs;
	int sid=data->sid;
	char *str=data->string;
	
	printf("Echo: %s\n", str);
	wCheck=write(sid, str, strlen(str));
	if (wCheck<0){
		perror("write\n");
		exit(-7);
	}
	memset( data->string, '\0', sizeof(data->string) );
	return NULL;
	/*
	int strIndex, wCheck;
	printf("Echo: ");
	for (strIndex=0; str[strIndex]!='\0'; strIndex++){
		printf("%c", str[strIndex]);
	}
	printf("\n");
	wCheck=write(sid, str, strlen(str));
	if (wCheck<0){
		perror("write\n");
		exit(-6);
	}
	*/
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
	
	listen(sockid, 0);
	
	//Accepts a client and calls the echo function
	int m; //tk
	while (1){
		//check if connection is accepted
		if((client=accept(sockid, (struct sockaddr *) &saddr, &addrlen))<=0){
			continue;
		}
		
		if (read(client, buf, 1024)<0){
			perror("read");
			exit(-4);
		}
		data_arr[0].sid=client;
		data_arr[0].string=buf;
		m = pthread_create(&threads[0], NULL, Echo, (void *) &data_arr[0]);
		if (m){
			perror("Pthread");
			exit(-5);
		}
	/*
		int m;
		m=fork();
		if (m<0){
			perror("fork");
			exit(-4);
		}
		if (m==0){
			//Child Process
			if (read(client, buf, 1024)<0){
				perror("read");
				exit(-5);
			}
			//close(sockid);
			echo(client, buf);
			exit(0);
		}
		else{
			//Parent process
			close(client);
		}
		*/
	}
	pthread_exit(NULL); //tk
	exit(0);

}
