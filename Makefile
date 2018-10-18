CC=g++
CFLAGS=-I. -std=c++0x
LIBS=-lpthread

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

all: server client

server: server.o
	$(CC) -o server server.o

client: client.o
	$(CC) -o client client.o $(LIBS)

