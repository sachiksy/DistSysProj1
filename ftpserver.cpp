#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

//Prints the message from the clients and writes the same message back to the client
void echo (int sid, char *str){
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
}

int main(int argc, char *argv[]){

	//kills zombie children
	signal(SIGCHLD, SIG_IGN);

	int sockid, client;
	unsigned int addrlen;
	char buf[1024];
	struct sockaddr_in saddr;
	
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
	addrlen=0;
	
	//Accepts a client and calls the echo function
	while ((client=accept(sockid, (struct sockaddr *) &saddr, &addrlen))>0){
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
	}
	
	exit(0);

}
