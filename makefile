CC=g++
CFLAGS=-Wall -pedantic
LDFLAGS=-lpthread -lm -ldl
# location of the Python header files
PYTHON_VERSION = 3.8.2
PYTHON_INCLUDE = /usr/include/python$(PYTHON_VERSION)
# location of the Boost Python include files and library
BOOST_INC = /usr/include
BOOST_LIB = /usr/lib

all: tracker client

tracker: tracker.cpp
	$(CC) -std=c++17 -o $@ $< $(CFLAGS) $(LDFLAGS)

client: peer.cpp
	$(CC) -std=c++17 -o $@ $< $(CFLAGS) $(LDFLAGS)

clean:
		rm client
		rm tracker
