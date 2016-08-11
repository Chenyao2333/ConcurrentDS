#include <cstdio>
#include <thread>
#include <mutex>
#include <climits>

struct Node {
    int key;
    bool marked;
    Node *next;
    std::mutex mtx;

    Node(int k = 0, Node *n = NULL) {
        key = k;
        marked = false;
        next = n;
    }
};

class LazyList {
public:
    LazyList() {
        Node *mx = new Node(INT_MAX);
        head = new Node(INT_MIN, mx);
    }

    bool Insert(int key) {
        while (true) {
            Node *curr = head;
            Node *next = curr->next;

            while (key > next->key) {
                curr = next;
                next = curr->next;
            }

            std::lock_guard<std::mutex> curr_lock(curr->mtx);
            std::lock_guard<std::mutex> next_lock(next->mtx);

            if (validate(curr, next)) {
                if (key == next->key) {
                    return false;
                }

                Node *np = new Node(key, next);
                curr->next = np;
                return true;
            }
        }
    }

    bool Find(int key) {
        Node *curr = head;
        while (curr and key > curr->key) {
            curr = curr->next;
        }
        return key == curr->key;
    }

    bool Delete(int key) {
        while (true) {
            Node *curr = head;
            Node *next = curr->next;

            while (key > next->key) {
                curr = next;
                next = curr->next;
            }

            std::lock_guard<std::mutex> curr_lock(curr->mtx);
            std::lock_guard<std::mutex> next_lock(next->mtx);

            if (validate(curr, next)) {
                if (key != next->key) {
                    return false;
                }

                next->marked = true;
                curr->next = next->next;
                return true;
            }
        }
    }
private:
    Node *head;

    bool validate(Node *curr, Node *next) {
        return !curr->marked and !next->marked and curr->next == next;
    }
};

typedef LazyList KV;
