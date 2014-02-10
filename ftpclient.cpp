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
#include <iostream>
#include <netdb.h>

using namespace std;

#define BUFFER 1024

int main(int argc, char *argv[]){
	int sock, i;
	struct addrinfo hints, *results, *j;
	char buf[BUFFER];
	
	//proper command args
	if (argc != 3){
		printf("Usage: %s <server hostname> <port number>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	//obtain address(es) matching hostname and port number
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;			//IPv4
	hints.ai_socktype = SOCK_STREAM;	//TCP socket
	
	//getaddrinfo(hostname, port number, address specifications, start of linked list of address structs)
	if (getaddrinfo(argv[1], argv[2], &hints, &results) < 0) {
		perror("Cannot resolve the address");
		exit(EXIT_FAILURE);
	}
	
	//try each address until successful connection
	for (j = results; j != NULL; j = j->ai_next) {
		//if socket creation fail, continue
		if ((sock = socket(j->ai_family, j->ai_socktype, j->ai_protocol)) < 0) {
			continue;
		}
		//connection success, break
		if (connect(sock, j->ai_addr, j->ai_addrlen) != -1) {
			break;
		}
		
		close(sock);
	}
	//no successful address
	if (j == NULL) {
		perror("Cannot connect\n");
		exit(EXIT_FAILURE);
	}
	
	freeaddrinfo(results);
	
	while(strcmp(buf, "exit")!=0){
		printf("myftp>");
		cin.getline(buf, BUFFER);
		i=write(sock, buf, strlen(buf));
		if (i<0){
			perror("write");
			exit(4);
		}
	
		i=read(sock, buf, BUFFER);
		if (i<0){
			perror("read");
			exit(5);
		}
	
		printf("Server reply: %s\n", buf);
	}
	
	close(sock);

	exit(0);

}
