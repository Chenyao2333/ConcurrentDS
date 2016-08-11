#include <cstdio>
#include <thread>

struct Node {
    int key;
    Node *next;
};

void test() {
    Node *lst = NULL;
    for (int i = 0; i < 1e6; i++) {
        Node *p = new Node();
        p->key = 12;
        p->next = lst;
        lst = p;
    }
}

int main() {
    std::thread ths[8];
    for (auto &th: ths) {
        th = std::thread(test);
    }

    for (auto &th: ths) {
        th.join();
    }
    return 0;
}
