CC = g++

hand_over_hand: hand_over_hand.cpp
	$(CC) -o $@ $^ -pthread -std=c++11

hoh_skiplist: hoh_skiplist.cpp
	$(CC) -g -o $@ $^ -pthread -std=c++11

lazy_list: test.cpp lazy_list.h
	$(CC) -o $@ $< -pthread -std=c++11 -DLAZY_LIST

michael_list: test.cpp michael_list.h hazard_pointer.h
	$(CC) -g -o $@ $< -pthread -std=c++11 -DMICHAEL_LIST

linked_list: test.cpp linked_list.h
	$(CC) -o $@ $< -pthread -std=c++11 -DLINKED_LIST

test_cas: test_cas.cpp
	$(CC) -o $@ $^ -pthread -std=c++11

test_hazard_point: test_hp.cpp hazard_pointer.h
	$(CC) -g -O2 -o $@ $< -pthread -std=c++11
