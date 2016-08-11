CC = g++

all: ctrie

ctrie: ctrie.cpp
	$(CC) -O2 -o $@ $< -lpthread -std=c++11

test_ctrie: ctrie
	./ctrie
