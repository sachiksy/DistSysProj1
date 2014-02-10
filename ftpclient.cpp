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
#include <fcntl.h>

using namespace std;

#define BUFFER 1024

const char eof[] = "EOF";

int main(int argc, char *argv[]){
	int sock, i;
	struct addrinfo hints, *results, *j;
	char buf[BUFFER];
	//char bloop[256];
	
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

	//user input handling
	while(strcmp(buf, "exit")!=0){
		printf("myftp> ");
		cin.getline(buf, BUFFER);
		
		// RECV/SEND for command: GET
		if (strstr(buf, "get")) {
			//send get <filename> to server
			i=write(sock, buf, strlen(buf));
			if (i<0){
				perror("write");
				exit(4);
			}

			//recv File status
			memset(buf, '\0', BUFFER);
			if (recv(sock, buf, BUFFER, 0) < 0){
				perror("error receiving file status from server");
				exit(EXIT_FAILURE);
			}
			printf("client received %s\n", buf);
			
			//file does not exist, print to user, loop back to prompt
			if (strstr(buf, "NULL")) {
				printf("Sorry, the file you wanted does not exist.\n");
			}
			
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
					}
					perror("Cannot open file to be written\n");
					close(sock);
					exit(EXIT_FAILURE);
				}
				//we are ready to receive
				else {
					openfile = "GOOD";
					//send GOOD TO GO, let's start sending that file
					if (send(sock, buf, sizeof(buf), 0) < 0) {
						perror("Cannot send ready to receive clearance to server\n");
						close(sock);
						exit(EXIT_FAILURE);
					}
					
					printf("opened file to write\n");
					while ((size = recv(sock, data, sizeof(data), 0)) > 0) {
					printf("receiving...\n");
						//if EOF, break
						if ((strcmp(data, eof)) == 0) {
							printf("end of file matey\n");
							break;
						}
						fwrite(data, 1, BUFFER, doc);
					}
					if (size < 0) {
						perror("problem receiving file from server");
						exit(EXIT_FAILURE);
					}
					printf("closing file\n");
					fclose(doc);
				}
			}
		}
		// WRITE/READ for commands: LS, PWD
		else {
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
		}
		printf("Server reply: %s\n", buf);
	}
	
	close(sock);

	exit(0);

}
