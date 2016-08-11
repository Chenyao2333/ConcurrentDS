CC = g++
FLAGS = -std=c++11 -O2

all: kvtester BIT_checker check_sample

hash_table_src = hashtable.h michael_list.h hazard_pointer.h

kvtester: kvtester.cpp $(hash_table_src)
	$(CC) -g -o $@ $< -pthread $(FLAGS)

test_tw: test_tw.cpp hashtable.h
	$(CC) -g -o $@ $< -pthread $(FLAGS)

BIT_checker: BIT_checker.cpp $(hash_table_src)
	$(CC) -o $@ $< $(FLAGS)

check_sample: check_sample.cpp $(hash_table_src)
	$(CC) -o $@ $< -pthread $(FLAGS)

clean:
	rm -f kvtester BIT_checker check_sample
