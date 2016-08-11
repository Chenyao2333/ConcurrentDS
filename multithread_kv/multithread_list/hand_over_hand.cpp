#include <cstdio>
#include <thread>
#include <mutex>
#include <set>
#include <vector>

struct Node {
    int key, val;
    Node *next;
    std::mutex mtx;

    Node(int k = 0, int v = 0): key(k), val(v) {
        next = NULL;
    }
};

class List {
private:
    Node *head;
public:
    List() {
        head = new Node(-2147483647);
        head->next = new Node(2147483647);
    }

    void Print() {
        Node *curr = head->next;

        while (curr) {
            printf("key = %d val = %d\n", curr->key, curr->val);
            curr = curr->next;
        }
    }

    void Insert(int key, int val) {
        Node *curr, *next;

        head->mtx.lock();
        curr = head;
        curr->next->mtx.lock();
        next = curr->next;

        while (key > next->key) {
            curr->mtx.unlock();
            curr = next;
            if (curr->next) {
                next = curr->next;
                next->mtx.lock();
            } else {
                break;
            }
        }

        if (key != next->key) {
            Node *newn = new Node(key, val);
            curr->next = newn;
            newn->next = next;
        } else {
            next->val = val;
        }

        curr->mtx.unlock();
        next->mtx.unlock();
    }

    int Query(int key) {
        Node *curr, *next;

        head->mtx.lock();
        curr = head;
        curr->next->mtx.lock();
        next = curr->next;

        while (key > next->key) {
            curr->mtx.unlock();
            curr = next;
            if (curr->next) {
                next = curr->next;
                next->mtx.lock();
            } else {
                break;
            }
        }

        int ret = 0;
        
        if (key == next->key) {
            ret = next->val;
        }

        curr->mtx.unlock();
        next->mtx.unlock();

        return ret;
    }
};

List lt;
void test() {
    std::set<int> s;
    std::vector<int> v;

    for (int i = 0; i <= 10000; i++) {
        int opt = rand() % 2;
        // printf("opt = %d\n", opt);
        if (opt == 0 and s.size()) {
            int key = v[rand() % v.size()];
            bool in_s = s.find(key) != s.end();
            bool in_lt = lt.Query(key) == -key;
            if (in_s != in_lt) {
                printf("WA\n");
                printf("key = %d lt_query = %d\n", key, lt.Query(key));
                exit(-1);
            }
        } else {
            int key = rand();
            lt.Insert(key, -key);
            s.insert(key);
            v.push_back(key);
        }
        // lt.Print();
    }
}

int main() {
    std::thread ths[4];
    for (auto &th: ths) {
        th = std::thread(test);
    }

    for (auto &th: ths) {
        th.join();
    }
    return 0;
}
