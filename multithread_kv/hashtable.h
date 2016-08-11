#ifndef HASHTABLE_H_
#define HASHTABLE_H_

// #include "michael_list.h"
#include "hazard_pointer.h"

int kBUCKET_SIZE = 100000 + 3;

struct Node {
    int key;
    int val;
    Node *next;

    Node (int k = 0, int v = 0, Node *n = NULL) {
        key = k;
        val = v;
        next = n;
    }

    bool cas(Node *old_next, Node *new_next) {
        return __sync_bool_compare_and_swap(&next, old_next, new_next);
    }
};

class HashTable {
public:
    /*
    HashTable() {
        bucket = std::vector<list_t>(kBUCKET_SIZE);
    }

    void Put(int key, int val) {
        int idx = hash_(key);
        bucket[idx].Put(key, val);
    }

    int Get(int key) {
        int idx = hash_(key);
        return bucket[idx].Get(key);
    }

    bool AtomicAdd(int key, int delta) {
        int idx = hash_(key);
        return bucket[idx].AtomicAdd(key, delta);
    }
    */


    class ThreadWoker {
        typedef HPRec<Node> hprec_t;
        hprec_t hprec;

        ThreadWorker() {
            hprec = hp.Allocate();
        }

        ~ThreadWorker() {
            hp.Release();
        }
    };

private:
    /*
    typedef MichaelList list_t;
    std::vector<list_t> bucket;
    */
    HazardPointer<Node> hp;

    int hash_(int key) {
        return key % kBUCKET_SIZE;
    }
};

#endif // hASHTABLE_H_
