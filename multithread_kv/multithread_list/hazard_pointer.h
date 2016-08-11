#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>
#include <algorithm>
#include <assert.h>

const int kK = 3;

#define CAS(p, old_val, new_val) __sync_bool_compare_and_swap(p, old_val, new_val)

template<class node_t>
struct HPRec {
    node_t *HP[kK];
    std::vector<node_t *> retire_list;
    bool active;

    HPRec<node_t> *next;

    HPRec() {
        for (int i = 0; i < kK; i++) HP[i] = NULL;
        active = false;
        next = NULL;
    }
};

thread_local static void *myhprec;

template<class node_t>
class HazardPointer {
public:
    HazardPointer() {
        hprec_head = NULL;
        H = 0;
    }

    void Allocate() {
        for (hprec_t *hprec = hprec_head; hprec; hprec = hprec->next) {
            if (hprec->active) continue;
            if (CAS(&(hprec->active), false, true)) {
                myhprec = hprec;
                return;
            }
        }

        int old_h = H;
        while (!CAS(&H, old_h, old_h + 1)) {
            old_h = H;
        }

        hprec_t *hprec = new hprec_t();
        hprec->active = true;
        myhprec = hprec;

        hprec_t *old_head = hprec_head;
        hprec->next = old_head;
        while (!CAS(&hprec_head, old_head, hprec)) {
            old_head = hprec_head;
            hprec->next = old_head;
        }
    }

    void Release() {
        for (int i = 0; i < kK; i++) ((hprec_t *)myhprec)->HP[i] = NULL;
        ((hprec_t *)myhprec)->active = false;
    }

    node_t *& operator [] (int i) {
        return ((hprec_t *)myhprec)->HP[i];
    }

    void RetireNode(node_t *ptr) {
        ((hprec_t *)myhprec)->retire_list.push_back(ptr);
        while (((hprec_t *)myhprec)->retire_list.size() >= 50*H*kK) {
            help_scan_();
            scan_(myhprec);
        }
    }

private:
    typedef HPRec<node_t> hprec_t;
    hprec_t *hprec_head;
    int H;

    void scan_(hprec_t *retired_hp) {
        std::vector<node_t *> hp_list;
        // int cnt = 0;
        for (hprec_t *hprec = hprec_head; hprec; hprec = hprec->next) {
            for (int i = 0; i < kK; i++) {
                node_t *ptr = hprec->HP[i];
                if (ptr) {
                    // cnt++;
                    hp_list.push_back((node_t *)ptr);
                }
            }
        }

        // printf("scan cnt = %d\n", cnt);

        std::sort(hp_list.begin(), hp_list.end());
        std::vector<node_t *> tmp_list;
        for (auto rp: retired_hp->retire_list) {
            auto it = std::lower_bound(hp_list.begin(), hp_list.end(), rp);
            if (it != hp_list.end() and *it == rp) {
                // printf("in hp\n");
                tmp_list.push_back((node_t *)rp);
            } else {
                node_t *hp_0 = NULL, *hp_1 = NULL;
                if (0 < hp_list.size()) hp_0 = hp_list[0];
                if (1 < hp_list.size()) hp_1 = hp_list[1];
                // printf("delete %p myhp = %p hp0 = %p hp1 = %p\n", rp, myhprec, hp_0, hp_1);
                delete rp;
            }
        }

        retired_hp->retire_list = tmp_list;
    }

    void help_scan_() {
        for (hprec_t *hprec = hprec_head; hprec; hprec = hprec->next) {
            if (hprec->active) continue;
            if (!CAS(&(hprec->active), false, true)) continue;

            for (auto rp: hprec->retire_list) {
                ((hprec_t *)myhprec)->retire_list.push_back(rp);
            }

            hprec->retire_list.clear();
            hprec->active = false;
        }
    }
};

