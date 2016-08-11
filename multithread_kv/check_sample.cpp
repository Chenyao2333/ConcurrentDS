#include "hashtable.h"

#include <map>
#include <iostream>

const int kBUCKET_SIZE = 5;
const int kELE_NUM = 15;
const int kITER = 100000000;
const int kTEST_TIMES = 1;

void test() {
    HashTable<int, int, long long, kBUCKET_SIZE> ht;

    long long sum = 0;

    std::map<int, int> cnt;
    for (int i = 0; i < kELE_NUM; i++) {
        int k = i + 1;
        sum += k;
        cnt[k] = 0;
        ht.Put(k, k);
    }

    for (int i = 0; i < kITER; i++) {
        int k = ht.Sample();
        cnt[k] += 1;
    }

    for (auto &kv: cnt) {
        double exp_prob = kv.first * 1.0 / sum;
        double real_prob = kv.second * 1.0 / kITER;
        printf("key = %d expected probability = %.05lf real probability = %.05lf\n", kv.first, exp_prob, real_prob);
    }
}

int main() {
    for (int i = 1; i <= kTEST_TIMES; i++) {
        test();
    }
    return 0;
}
