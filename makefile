prog: ftpclient.c ftpserver.c
	gcc -g -Wall ftpserver.c -o ftpServer
	gcc -g -Wall ftpclient.c -o ftpClient
	
testserv:
	./ftpServer 1459
	
testcli:
	./ftpClient 127.0.0.1 1459
	
clean:
	rm ftpServer ftpClient