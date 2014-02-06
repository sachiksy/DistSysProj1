#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
using namespace std;

int main(int argc, char *argv[]){

	int sock, i;
	struct sockaddr_in saddr;
	char buf[1024];
	struct in_addr inp;

	if (argc!=3){
		printf("Usage: need a server IP address and server port number\n");
		exit(1);
	}
	
	inet_aton(argv[1], &inp);
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inp.s_addr;
	saddr.sin_port = htons(atoi(argv[2]));
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	if (sock<0){
		perror("socket");
		exit(2);
	}
	
	if (connect(sock, (const struct sockaddr *)&saddr, sizeof(saddr))<0){
		perror("connection");
		exit(3);
	}
	
	while(strcmp(buf, "exit")!=0){
		printf("myftp>");
		cin.getline(buf, 1024);
		i=write(sock, buf, strlen(buf));
		if (i<0){
			perror("write");
			exit(4);
		}
	
		i=read(sock, buf, 1024);
		if (i<0){
			perror("read");
			exit(5);
		}
	
		printf("Server reply: %s\n", buf);
	}
	
	close(sock);

	exit(0);

}
