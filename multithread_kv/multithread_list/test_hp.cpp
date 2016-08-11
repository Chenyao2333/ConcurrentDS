#include "hazard_pointer.h"

struct Node {
    int key;

    Node (int k = 0) {
        key = k;
    }
};

HazardPointer<Node> hp;
Node *t[10];

volatile bool ready = false;
void test() {
    while (!ready);

    hp.Allocate();
    while (true) {
        int op = rand() % 2;
        int id = rand() % 10;
        
        hp[0] = t[id];
        if (hp[0] != t[id]) continue;
        if (hp[0]) {
            if (rand() % 2) {
                if (CAS(&t[id], hp[0], NULL)) {
                    hp.RetireNode(hp[0]);
                } else {
                    // printf("t[%d] has changed\n", id);
                }
            }

            if (hp[0]->key != id + 1) {
                printf("WA hp[0] = %p hp[0]->key = %d id = %d myhp = %p\n", hp[0], hp[0]->key, id, myhprec);
                exit(-1);
            }

            hp[0] = NULL;
        } else {
            Node *newn = new Node(id + 1);
            if (!CAS(&t[id], NULL, newn)) {
                delete newn;
            } else {
                // printf("insert %p myhp = %p\n", newn, myhprec);
            }
        }
    }
    hp.Release();
}

int main() {
    std::thread ths[2];
    for (auto &th: ths) {
        th = std::thread(test);
    }
    ready = true;

    for (auto &th: ths) {
        th.join();
    }
    return 0;
}

