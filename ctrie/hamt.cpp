#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <assert.h>

#include <set>
#include <vector>

using namespace std;

const int kBRANCH_WIDTH = 5;

class CTrie {
public:
    CTrie() {
        root = new CNode();
    };

    ~CTrie() {
        // TODO: release memory to prevent memory leak
    }

    bool Insert(int key) {
        CNode *p = root;
        int lev = -1;
        while (true) {
            lev++;
            int pos = get_pos(key, lev);
            bool is_cnode = p->is_cnode_bmp & (1 << pos);

            if (p->branch[pos]) {
                if (is_cnode) {
                    p = (CNode *) p->branch[pos];
                } else {
                    SNode *sn = (SNode *) p->branch[pos];
                    if (sn->key == key) {
                        return false;
                    }

                    CNode *ncn = new CNode();
                    ncn->update(get_pos(sn->key, lev+1), false, sn); // level will not exceed the maximum level due to the key is not equal sn's key.
                    p->update(pos, true, ncn);
                    p = ncn;
                }
            } else {
                SNode *sn = new SNode(key);
                p->update(pos, false, sn);
                return true;
            }
        }
    }

    bool Find(int key) {
        CNode *p = root;
        int lev = -1;
        while (true) {
            lev++;
            int pos = get_pos(key, lev);
            bool is_cnode = p->is_cnode_bmp & (1 << pos);

            if (!p->branch[pos]) return false;

            if (is_cnode) {
                p = (CNode *) p->branch[pos];
            } else {
                SNode *sn = (SNode *) p->branch[pos];
                return sn->key == key;
            }
        }
    }

    bool Delete(int key) {
        CNode *p = root;
        int lev = -1;
        CNode *path[80 / kBRANCH_WIDTH];
        while (true) {
            lev++;
            int pos = get_pos(key, lev);
            bool is_cnode = p->is_cnode_bmp & (1 << pos);
            
            if (!p->branch[pos]) return false;
            
            if (is_cnode) {
                path[lev] = p;
                p = (CNode *) p->branch[pos];
            } else {
                SNode *sn = (SNode *) p->branch[pos];
                if (sn->key != key) return false;
                delete sn;
                p->update(pos, false, NULL);
                path[lev] = p;

                clear_path(path, lev, key);
                return true;
            }
        }
    }

private:
    struct BasicNode {
    };

    struct CNode: BasicNode {
        // If branch[i] is null, it respent there isn't a son node at branch[i]
        // Otherwish it's a son node and the corresponding bit in is_node_bmp respent
        // whether it's a CNode.
        int is_cnode_bmp;
        BasicNode *branch[1 << kBRANCH_WIDTH];

        CNode() {
            memset(branch, 0, sizeof(branch));
        }

        void update(int pos, bool is_cnode, BasicNode *n) {
            branch[pos] = n;
            is_cnode_bmp = is_cnode_bmp & (~(1 << pos)) | int(is_cnode) << pos;
        }
    };

    struct SNode: BasicNode {
        int key;

        SNode() {
        }

        SNode(int k) {
            key = k;
        }
    };

    CNode *root;

    int get_pos(int key, int lev) {
        key >>= lev * kBRANCH_WIDTH;
        return key & ((1 << kBRANCH_WIDTH) - 1);
    }

    bool contracted(CNode *p, SNode *&tsn) {
        if (p->is_cnode_bmp) return false;
        tsn = NULL;
        for (int i = 0; i < (1 << kBRANCH_WIDTH); i++) {
            if (p->branch[i]) {
                if (tsn) return false;
                else tsn = (SNode *) p->branch[i];
            }
        }

        delete p;
        return true;
    }

    void clear_path(CNode *path[], int lev, int key) {
        CNode *p = path[lev];
        SNode *tsn = NULL;
        while (lev and contracted(p, tsn)) {
            // printf("lev = %d\n", lev);
            p = path[--lev];
            p->update(get_pos(key, lev), false, tsn);
        }
    }
};


void basic_test() {
    CTrie ct;
    int arr[] = {1, 2, 5, 32768, 65536};
    int del[] = {65536};
    int test[] = {1, 4, 5, 7, 32768, 65536};

    for (int n: arr) {
        printf("Insert %d\n", n);
        assert(ct.Insert(n) == true);
    }

    for (int n: del) {
        printf("Delete %d\n", n);
        assert(ct.Delete(n) == true);
    }

    for (int n: test) {
        if (ct.Find(n)) {
            printf("%d is in set\n", n);
        } else {
            printf("%d is not in set\n", n);
        }
    }
}

void test_insfind() {
    CTrie ct;
    set<int> s;
    vector<int> l;
    for (int i = 0; i < 1e5; i++) {
        int op = rand() % 10;
        if (op <= 7) op = op % 2;
        else op = 2;
        // printf("Testing #%d\n", i);
        switch (op) {
            case 0: {
                int n = rand();
                if (l.size() and rand() % 2) {
                    n = l[rand() % l.size()];
                }

                // printf("Finding %d\n", n);
                assert((s.find(n) != s.end()) == ct.Find(n));
                break;
            }
            case 1: {
                int n = rand();
                if (l.size() and rand() % 10 == 0) {
                    n = l[rand() % l.size()];
                }

                // printf("Inserting %d\n", n);
                if (s.find(n) != s.end()) {
                    assert(ct.Insert(n) == false);
                } else {
                    s.insert(n);
                    assert(ct.Insert(n));
                }
                break;
            }
            case 2: {
                int n = rand();
                if (l.size() and rand() % 7) {
                    n = l[rand() % l.size()];
                }

                if (s.find(n) != s.end()) {
                    s.erase(n);
                    assert(ct.Delete(n));
                } else {
                    assert(!ct.Delete(n));
                }
            }
        }
    }

    printf("Finshed batch test!\n");
}

int main() {
    //basic_test();
    test_insfind();
    return 0;
}
