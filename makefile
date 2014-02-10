prog: ftpclient.cpp ftpserver.cpp
	g++ ftpserver.cpp -Wall -lpthread -o ftpServer
	g++ ftpclient.cpp -Wall -o ftpClient
	
ser:
	./ftpServer 1459
	
cli:
	./ftpClient cf0.cs.uga.edu 1459
	
clean:
	rm ftpServer ftpClient
