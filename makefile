prog: ftpclient.cpp ftpserver.cpp
	g++ ftpserver.cpp -Wall -lpthread -o ftpServer
	g++ ftpclient.cpp -Wall -o ftpClient
	
testserv:
	./ftpServer 1459
	
testcli:
	./ftpClient 127.0.0.1 1459
	
clean:
	rm ftpServer ftpClient