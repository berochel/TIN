CC=g++ -std=c++17
CFLAGS=-Wall -pedantic
LDFLAGS=-lpthread -lm -ldl

all: tracker client

tracker: tracker.cpp
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

client: peer.cpp
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

clean:
		rm client tracker
