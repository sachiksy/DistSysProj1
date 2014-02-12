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
#include <netdb.h>
#include <fcntl.h>

using namespace std;

#define BUFFER 1024

const char eof[] = "EOF";

int main(int argc, char *argv[]){
	int sock, i;
	//struct sockaddr_in saddr;
	struct addrinfo hints, *results, *j;
	char buf[BUFFER];
	//struct in_addr inp;

	//check for proper command args
	if (argc!=3){
		printf("Usage: %s <server hostname> <port number>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	/*
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
	} */
	
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
	
	//user input handling
	while(strcmp(buf, "exit") != 0){
		printf("myftp> ");
		cin.getline(buf, BUFFER);
		
		// RECV/SEND for command: GET
		if (strstr(buf, "get")) {
			//send get <filename> to server
			i=write(sock, buf, strlen(buf));
			if (i<0){
				perror("write");
				exit(4);
			} //if (i<0)

			//recv File status
			memset(buf, '\0', BUFFER);
			if (recv(sock, buf, BUFFER, 0) < 0){
				perror("error receiving file status from server");
				exit(EXIT_FAILURE);
			} //if (recv(sock, buf, BUFFER, 0) < 0)
			printf("client received %s\n", buf);
			
			//file does not exist, print to user, loop back to prompt
			if (strstr(buf, "NULL")) {
				printf("Sorry, the file you wanted does not exist.\n");
			} //if (strstr(buf, "NULL"))
			
			//file does exist, create data socket/bind/listen/accept, send connection status
			else {
				printf("client knows %s EXISTS\n", buf);
				
				size_t size;
				char data[BUFFER];
				memset(data, 0, sizeof(data));
				//overwrite existing file or create new file
				FILE* doc = fopen(buf, "wb");
				
				char *openfile;
				//cannot open file to write
				if (doc == NULL) {
					openfile = "CANT";
					//send file status, end
					if (send(sock, buf, sizeof(buf), 0) < 0) {
						perror("Cannot send file opening status to server\n");
						close(sock);
						exit(EXIT_FAILURE);
					} //if (send(sock, buf, sizeof(buf), 0) < 0)
					perror("Cannot open file to be written\n");
					close(sock);
					exit(EXIT_FAILURE);
				} //if (doc == NULL)
				//we are ready to receive
				else {
					openfile = "GOOD";
					//send GOOD TO GO, let's start sending that file
					if (send(sock, buf, sizeof(buf), 0) < 0) {
						perror("Cannot send ready to receive clearance to server\n");
						close(sock);
						exit(EXIT_FAILURE);
					} //if (send(sock, buf, sizeof(buf), 0) < 0)
					
					printf("opened file to write\n");
					while ((size = recv(sock, data, sizeof(data), 0)) > 0) {
					printf("receiving...\n");
						//if EOF, break
						if ((strcmp(data, eof)) == 0) {
							printf("end of file matey\n");
							break;
						} //if ((strcmp(data, eof)) == 0)
						fwrite(data, 1, BUFFER, doc);
					} //while ((size = recv(sock, data, sizeof(data), 0)) > 0)
					if (size < 0) {
						perror("problem receiving file from server");
						exit(EXIT_FAILURE);
					} //if (size < 0)
					printf("closing file\n");
					fclose(doc);
				} //else
			} //else
		} //if (strstr(buf, "get"))
		// WRITE/READ for commands: LS, PWD
		else {
			i=write(sock, buf, strlen(buf));
			if (i<0){
				perror("write");
				exit(4);
			} //if (i<0)

			i=read(sock, buf, BUFFER);
			if (i<0){
				perror("read");
				exit(5);
			} //if (i<0)
		} //else
		printf("Server reply: %s\n", buf);
	} //while(strcmp(buf, "exit")!=0)
	
	close(sock);

	exit(0);

}
