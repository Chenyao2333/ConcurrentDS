#ifndef MICHAEL_LIST_
#define MICHAEL_LIST_

#include "hazard_pointer.h"

#include <cstdio>
#include <algorithm>
#include <thread>
#include <climits>

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


typedef unsigned long long ULL;
#define GET_NEXT(next) (Node *)(ULL(next) & ~ULL(1))
#define IS_MARK(next) bool(ULL(next) & 1)
#define SET_MARK(next) (Node *)(ULL(next) | 1)

class MichaelList {
public:
    MichaelList() {
        Node *newn = new Node(INT_MAX);
        head = new Node(INT_MIN, 0, newn);
        // printf("MichaelList is initializing");
    }

    void Put(int key, int val) {
        Node *newn = new Node(key, val);
        while (true) {
            auto p_next = _find(key);
            Node *p = p_next.first, *next = p_next.second;

            if (key == next->key) {
                next->val = val;
                delete newn;
                return;
            }

            newn->next = next;
            if (p->cas(next, newn)) {
                return;
            } else {
                // printf("CAS filed\n");
            }
        }
    }

    int Get(int key) {
        auto p_next = _find(key);
        if (p_next.second->key == key) {
            return p_next.second->val;
        } else {
            return 0;
        }
    }

    bool AtomicAdd(int key, int delta) {
        while (true) {
            auto p_next = _find(key);
            Node *p = p_next.first, *next = p_next.second;
            if (next->key != key) {
                return false;
            }

            int old_val = next->val;
            int new_val = old_val + delta;
            if (new_val == 0) {
                // try to delete it!
                Node *new_next = next->next;
                if (IS_MARK(new_next)) return false;

                if (next->cas(new_next, SET_MARK(new_next))) {
                    if (p->cas(next, new_next)) {
                        hp.RetireNode(next);
                    }
                    return false;
                }
            } else {
                while (!CAS(&(next->val), old_val, new_val)) {
                    old_val = next->val;
                    new_val = old_val + delta;
                }
                return true;
            }
        }
    }

    bool Delete(int key) {
        while (true) {
            auto p_next = _find(key);
            Node *p = p_next.first, *next = p_next.second;

            if (key != next->key) return false;

            Node *new_next = next->next;
            if (IS_MARK(new_next)) return false; // next has been deleted

            // It fails when a new node has been inserted after 'next' or 'next' has been deleted by another thread. Although the second scenario is unsual.
            // The _find guarantees the p and next isn't marked, but it's have possbility that it's be marked by others.
            if (next->cas(new_next, SET_MARK(new_next))) {
                if (p->cas(next, new_next)) {
                    hp.RetireNode(next);
                }
                return true;
            } else {
                // printf("cas failed!\n");
            }
        }
    }

private:
    // return two nodes [n1, n2]  which satisfy n1.key < key and key <= n2.key
    std::pair<Node *, Node *> _find(int key) {
        while (true) {
            Node *p = head;
            Node *next = GET_NEXT(p->next);

            hp[0] = p;
            hp[1] = next;
            if (next != GET_NEXT(p->next)) continue;

            while (true) {
                Node *new_next = next->next;

                hp[2] = GET_NEXT(new_next);
                if (next->next != new_next) continue;

                if (IS_MARK(new_next)) { // next is marked!
                    new_next = GET_NEXT(new_next); // clear mark 
                    if (p->cas(next, new_next)) {
                        hp.RetireNode(next);
                        hp[1] = new_next;
                        next = new_next;
                    } else {
                        break; // try again. maybe a new node be inserted between p and next or p is deleted.
                    }
                } else {
                    if (key <= next->key) {
                        return std::make_pair(p, next);
                    }
                    p = next;
                    next = new_next;
                    hp[0] = p;
                    hp[1] = next;
                }
            }
        }
    }
    Node *head;
};

#endif
