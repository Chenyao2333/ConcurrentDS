#include <cstdio>
#include <climits>

struct Node {
    int key;
    Node *next;

    Node (int k = 0, Node *n = NULL) {
        key = k;
        next = n;
    }
};


struct LinkedList {
public:
    LinkedList() {
        Node *end = new Node(INT_MAX);
        head = new Node(INT_MIN, end);
    }

    bool Find(int key) {
        Node *p = head;
        Node *next = p->next;

        while (key > next->key) {
            p = next;
            next = p->next;
        }

        return key == next->key;
    }

    bool Delete(int key) {
        Node *p = head;
        Node *next = p->next;

        while (key > next->key) {
            p = next;
            next = p->next;
        }
        
        if (key == next->key) {
            p->next = next->next;
            // delete next;
            return true;
        } else {
            return false;
        }
    }

    bool Insert(int key) {
        Node *p = head;
        Node *next = p->next;

        while (key > next->key) {
            p = next;
            next = p->next;
        }

        if (key == next->key) return false;
        Node *newn = new Node(key, next);
        p->next = newn;
        return true;
    }

private:
    Node *head;
};

typedef LinkedList KV;
