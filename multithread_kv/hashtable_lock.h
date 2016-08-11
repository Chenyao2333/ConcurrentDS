#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#define SAMPLE_F
#define THREAD_SAFE_F

#include <algorithm>
#include <iostream>
#include <vector>
#include <mutex>

/*
template <class V>
V rand(V lower, V upper) {
    V ret = 0;
    ret = rand() | (rand() << 16) | (rand() << 32) | (rand() << 64);
    if (ret < 0) ret = -ret;
    ret = ret * ((long long)(ret / (max_val - min_val)) - 1);
}
*/

template <class K, class V>
struct KVItem {
    K key;
    V val;

    KVItem();
    KVItem(K key, V val): key(key), val(val) {}
};

#ifdef SAMPLE_F
template <class V>
class BinaryIndexTree {
  public:
    BinaryIndexTree(size_t max_range) { // the valid operation range in [0, max_range]
/*
#ifdef THREAD_SAFE_F
        mutexes = std::vector<std::mutex>(max_range + 2);
#endif
*/
        tree.resize(max_range + 2);
        tree_size = tree.size();
        sum = 0;
    }

    BinaryIndexTree(const BinaryIndexTree &other) {
        // std::lock_guard<std::mutex> lock_guard(other.m);
        tree = other.tree;
        tree_size = other.tree_size;
        sum = other.sum;
    }

    void Add(int pos, V delta) {
#ifdef THREAD_SAFE_F
        std::lock_guard<std::mutex> lock_guard(m);
#endif
        pos += 1; // beacuse 0 is undeined in BIT, for all operation we just add 1 to the positation
        sum += delta;
        for (int i = pos; i < tree_size; i += lowbit(i)) {
/*
#ifdef THREAD_SAFE_F
            std::lock_guard<std::mutex> lock_guard(mutexes[i]);
#endif
*/
            tree[i] += delta;
        }
    }

    V get_sum() {
        return sum;
    }
    /*
     * Find the last element which prefix summary is less than val;
     *     return this elements index and the prefix summary at it, if can't find it, return (-1, 0)
     */
    std::pair<int, V> last_less(V val) {
        int p = 0;
        V sum = V();
        for (int i = 30; i >= 0; i--) {
            int newp = p | (1 << i);
            if (newp < tree_size) {
                // printf("newp = %d tree[%d] = %d\n", newp, newp, tree[newp]);
                if (sum + tree[newp] < val) {
                    p = newp;
                    sum += tree[newp];
                }
            }
        }

        p--;
        return std::make_pair(p, sum);
    }

  private:
    typedef std::vector<V> tree_t;

#ifdef THREAD_SAFE_F
    std::mutex m;
    // std::vector<std::mutex> mutexes;
#endif

    tree_t tree;
    size_t tree_size;
    V sum;

    int lowbit(int x) { return x&-x; }
};
#endif // SAMPLE_F

template <class K, class V, class BIT_ELE_t, size_t BUCKET_SIZE>
class HashTable {
  public:
    HashTable() {
        bucket.resize(BUCKET_SIZE);
    }

    void Put(K key, V val) {
        size_t bucket_ind = CalcHash(key);
#ifdef THREAD_SAFE_F
        std::lock_guard<std::mutex> lock_guard(mutexes[bucket_ind]);
#endif

        for (kv_t &kv: bucket[bucket_ind]) {
            if (kv.key == key) {
#ifdef SAMPLE_F
                BIT.Add(bucket_ind, val - kv.val);
#endif
                kv.val = val;
                return;
            }
        }

        // not existing
        bucket[bucket_ind].push_back(kv_t(key, val));
#ifdef SAMPLE_F
        BIT.Add(bucket_ind, val);
#endif
    }

    V Get(K key) {
        size_t bucket_ind = CalcHash(key);
#ifdef THREAD_SAFE_F
        std::lock_guard<std::mutex> lock_guard(mutexes[bucket_ind]);
#endif

        for (kv_t &kv: bucket[bucket_ind]) {
            if (kv.key == key) {
                return kv.val;
            }
        }
        return V();
    }

    void AtomicAdd(K key, V delta) {
        size_t bucket_ind = CalcHash(key);
#ifdef THREAD_SAFE_F
        std::lock_guard<std::mutex> lock_guard(mutexes[bucket_ind]);
#endif

        list_t &list_ = bucket[bucket_ind];

        for (auto it = list_.begin(); it != list_.end(); ++it) {
            if (it->key == key) {
                it->val += delta;
#ifdef SAMPLE_F
                BIT.Add(bucket_ind, delta);
#endif
                if (it->val == 0) {
                    list_.erase(it);
                }
                break;
            }
        }
    }

    K Sample() {
        V mark = rand() % BIT.get_sum() + 1;
        auto res = BIT.last_less(mark);
        // printf("mark = %d res.first = %d res.second = %d\n", mark, res.first, res.second);

        int ind = res.first + 1;
        mark -= res.second;

        for (auto &kv: bucket[ind]) {
            mark -= kv.val;
            if (mark <= 0) {
                return kv.key;
            }
        }

        return -1;
    }

  private:
    typedef KVItem<K, V> kv_t;
    typedef std::vector<kv_t> list_t;
    typedef std::vector<list_t> bucket_t;

    // list_t bucket[BUCKET_SIZE];
    bucket_t bucket;
#ifdef SAMPLE_F
    BinaryIndexTree<BIT_ELE_t> BIT = BinaryIndexTree<BIT_ELE_t>(BUCKET_SIZE);
#endif
#ifdef THREAD_SAFE_F
    std::vector<std::mutex> mutexes = std::vector<std::mutex>(BUCKET_SIZE);
#endif

    size_t CalcHash(K key) {
        // return key % BUCKET_SIZE;
        size_t hash_val = 0;
        char *p = (char *)&key;
        for (int i = 0; i < sizeof(key); ++p, ++i) {
            hash_val ^= (hash_val << 4) ^ (size_t)(*p);
        }
        return hash_val % BUCKET_SIZE;
    }
};

#endif // HASHTABLE_H_
