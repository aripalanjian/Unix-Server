CFLAGS = -g -Wall #-std=c++11
CC = g++

all: server client

server_objects = main.o server.o

server: $(server_objects)
	$(CC) $(CFLAGS) -o server $(server_objects) 

main.o: main.cpp

server.o: server.cpp

client_objects = client.o

client: $(client_objects)
	$(CC) $(CFLAGS) -o client $(client_objects)

client.0: client.cpp

.PHONY : clean all
clean: 
	rm -f server $(server_objects)
	rm -f client $(client_objects)
	rm -f errReport.txt
	rm -f clientErrReport.txt
	rm -f serverErrReport.txt
	rm -f serverlog.txt
	rm -f serverlog_win.txt