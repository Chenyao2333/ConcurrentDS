#include <cstdio>
#include <thread>
#include <atomic>

const int kIT_CNT = 1e7;

// int sum = 0;
std::atomic<int> sum(0);

void add() {
    for (int i = 0; i < kIT_CNT; i++) {
        while (true) {
            int s = sum;
            if (std::atomic_compare_exchange_strong(&sum, &s, s+1)) {
                break;
            }
            /*
            if (__sync_bool_compare_and_swap(&sum, s, s+1)) {
                break;
            }
            */
        }
    }
}

void test1() {
    std::thread ths[4];
    for (auto &th: ths) {
        th = std::thread(add);
    }

    for (auto &th: ths) {
        th.join();
    }

    printf("sum = %d\n", sum.load());
} 

void test2() {
    int *n = new int(5);
    bool b1 = __sync_bool_compare_and_swap(n, 5, 6);
    delete n;
    printf("n = %d\n", *n);
    bool b2 = __sync_bool_compare_and_swap(n, 6, 7);
    bool b3 = __sync_bool_compare_and_swap(n, 6, 7);
    printf("%d %d %d\n", (int)b1, (int)b2, (int)b3);
}

struct Node {
    int key;
    int mark;
    Node() {
        key = 0;
        mark = false;
    }
};

void test3() {
    Node a, b;
    b.key = 1;
    bool b1 = __sync_bool_compare_and_swap((void *)a, a, b);
    printf("%d\n", (int)b1);
}
int main() {
    // test1();
    // test2();
    test3();
    return 0;
}
