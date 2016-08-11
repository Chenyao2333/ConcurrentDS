#include <cstdio>
#include <thread>
#include <mutex>
#include <atomic>
#include <set>
#include <vector>

const int kMaxLevel = 20;

class SpinLock
{
public:
    void lock()
    {
        while(lck.test_and_set(std::memory_order_acquire))
        {}
    }

    void unlock()
    {
        lck.clear(std::memory_order_release);
    }

private:
    std::atomic_flag lck = ATOMIC_FLAG_INIT;
};

template <class K>
struct Node {
    Node *down, *next;
    K key;
    // std::mutex mtx;
    SpinLock mtx;

    Node(K k = 0) {
        key = k;
        down = next = NULL;
    }
};

int rand_level() {
    int level = 0;
    while (level < kMaxLevel and rand() % 2) level++;
    return level;
}

template <class K>
class SkipList {
public:
    SkipList() {
        head = new Node_t();
        Node_t *p = head;
        for (int i = 1; i < kMaxLevel; i++) {
            Node_t *newn = new Node_t;
            p->down = newn;
            p = newn; 
        }
    }

    bool Find(K key) {
        head->mtx.lock();
        Node_t *p = head;
        Node_t *next = p->next;
        if (next) next->mtx.lock();

        while (p) {
            while (next and key > next->key) {
                p->mtx.unlock();
                p = next;
                next = p->next;
                if (next) next->mtx.lock();
            }

            bool found = next and next->key == key;
            if (found) {
                p->mtx.unlock();
                if (next) next->mtx.unlock();
                return true;
            }

            Node_t *newp = p->down;
            if (newp) newp->mtx.lock();

            p->mtx.unlock();
            if (next) next->mtx.unlock();

            if (!newp) break;
            p = newp;
            next = p->next;
            if (next) next->mtx.lock();
        }

        return false;
    }

    void Insert(K key) {
        head->mtx.lock();
        Node_t *p = head;
        Node_t *next = p->next;
        if (next) next->mtx.lock();
        
        Node_t *prec[kMaxLevel];
        int new_level = rand_level();

        for (int i = kMaxLevel - 1; i >= 0; i--) {
            while (next and key > next->key) {
                p->mtx.unlock();
                p = next;
                next = p->next;
                if (next) next->mtx.lock();
            }
            if (next) next->mtx.unlock();

            prec[i] = p;
            if (i) {
                p = p->down;
                p->mtx.lock();
                next = p->next;
                if (next) next->mtx.lock();
            }

            if (i > new_level) prec[i]->mtx.unlock();
        }

        Node_t *down = NULL;
        for (int i = 0; i <= new_level; i++) {
            Node_t *newn = new Node_t(key);
            newn->next = prec[i]->next;
            newn->down = down;
            prec[i]->next = newn;
            down = newn;
            prec[i]->mtx.unlock();
        }
    }
private:
    typedef Node<K> Node_t;
    Node_t *head;
};

SkipList<long long> lt;
volatile bool ready = false;
void test() {
    std::set<long long> s;
    std::vector<long long> v;

    int this_id = std::hash<std::thread::id>()(std::this_thread::get_id());
    
    while (!ready);

    for (int i = 0; i <= 1000000; i++) {
        int opt = rand() % 2;
        if (opt and s.size()) {
            long long key = v[rand() % v.size()];
            if (rand() % 10) key = rand();
            // printf("query %lld\n", key);
            bool in_s = s.find(key) != s.end();
            bool in_lt = lt.Find(key);
            if (in_s != in_lt) {
                printf("key = %lld in_lt = %d lt_find = %d id = %d\n", key, int(in_lt), int(lt.Find(key)), this_id);
                printf("WA\n");
                exit(-1);
            }
        } else {
            long long key = ((long long)(rand()) << 30) ^ rand() ;
            // printf("insert %lld id = %d\n", key, this_id);
            lt.Insert(key);
            s.insert(key);
            v.push_back(key);
        }
        // lt.Print();
    }
}

void test_thoughput() {
    std::vector<long long> v;
    while (!ready);
    for (int i = 0; i <= 500000; i++) {
        int opt = rand() % 2;
        if (opt and v.size()) {
            long long key = v[rand() % v.size()];
            lt.Find(key);
        } else {
            long long key = ((long long)(rand()) << 30) ^ rand();
            lt.Insert(key);
        }
    }
}

int main() {
    std::thread ths[1];
    for (auto &th: ths) {
        th = std::thread(test);
    }
    ready = true;

    for (auto &th: ths) {
        th.join();
    }
    return 0;
}
