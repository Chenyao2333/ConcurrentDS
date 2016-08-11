#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <thread>
#include <assert.h>

#include <set>
#include <vector>
#include <ctime>

#define MEASURE_COST

using namespace std;

const int kBRANCH_WIDTH = 5;

class CTrie {
public:
    CTrie() {
        root = new INode();
        root->main = new CNode();
    };

    ~CTrie() {
        // TODO: release memory to prevent memory leak
    }

    bool Insert(int key) {
        while (true) {
            INode *in = root;
            INode *par = NULL;
            int lev = -1;

            while (true) {
                lev++;
                CNode *p = (CNode *) in->main;
                if (p->node_type == TNODE) {
                    clear(par, lev-1);
                    break;
                }

                int pos = get_pos(key, lev);
                bool is_cnode = p->is_cnode_bmp & (1 << pos);
                
                if (p->branch[pos]) {
                    if (is_cnode) {
                        par = in;
                        in = (INode *) p->branch[pos];
                    } else {
                        SNode *sn = (SNode *) p->branch[pos];
                        if (sn->key == key) return false;
                        SNode *nsn = new SNode(key);
                        CNode *ncn = new CNode();
                        *ncn = *p;
                        INode *nin = new INode(new CNode(sn, nsn, lev+1));

                        ncn->update(pos, true, nin);

                        if (in->cas_main(p, ncn)) {
                            // delete p by hazard pointer!
                            return true;
                        } else {
                            delete ncn;
                            break;
                        }
                    }
                } else {
                    SNode *sn = new SNode(key);
                    CNode *ncn = new CNode();
                    *ncn = *p;
                    ncn->update(pos, false, sn);

                    if (in->cas_main(p, ncn)) {
                        // delete p by hazard pointer;
                        return true;
                    } else {
                        delete ncn;
                        break;
                    }
                }
            }
        }
    }

    bool Find(int key) {
        while (true) {
            INode *in = root;
            INode *par = NULL;
            int lev = -1;
            while (true) {
                lev++;
                CNode *p = (CNode *) in->main;
                if (p->node_type == TNODE) {
                    clear(par, lev-1);
                    break;
                }

                int pos = get_pos(key, lev);
                bool is_cnode = p->is_cnode_bmp & (1 << pos);

                if (!p->branch[pos]) return false;

                if (is_cnode) {
                    par = in;
                    in = (INode *) p->branch[pos];
                } else {
                    SNode *sn = (SNode *) p->branch[pos];
                    return sn->key == key;
                }
            }
        }
    }

    bool Delete(int key) {
        while (true) {
            INode *in = root;
            INode *par = NULL;
            int lev = -1;
            while (true) {
                lev++;
                CNode *p = (CNode *) in->main;
                if (p->node_type == TNODE) {
                    clear(par, lev-1);
                    break;
                }

                int pos = get_pos(key, lev);
                bool is_cnode = p->is_cnode_bmp & (1 << pos);
                
                if (!p->branch[pos]) return false;

                if (is_cnode) {
                    par = in;
                    in = (INode *) p->branch[pos];
                } else {
                    SNode *sn = (SNode *) p->branch[pos];
                    if (sn->key != key) return false;

                    CNode *ncn = new CNode();
                    *ncn = *p;
                    ncn->update(pos, false, NULL);

                    SNode *tsn = new SNode();
                    if (to_contracted(ncn, tsn)) {
                        if (in->cas_main(p, tsn)) {
                            clear(par, lev-1);
                            return true;
                        } else {
                            break;
                        }
                    } else {
                        if (in->cas_main(p, ncn)) {
                            return true;
                        } else {
                            break;
                        }
                    }
                }
            }
        }
    }

private:
    static const int BASICNODE = 0;
    static const int SNODE = 1;
    static const int CNODE = 2;
    static const int INODE = 3;
    static const int TNODE = 4;

    struct BasicNode {
        int node_type;

        BasicNode() {
            node_type = BASICNODE;
        }
    };

    struct SNode: BasicNode {
        int key;

        SNode(int k = 0) {
            key = k;
            node_type = SNODE;
        }

        void entomb() {
            node_type = TNODE;
        }
        
        void resurrect() {
            node_type = SNODE;
        }
    };
    
    struct CNode: BasicNode {
        // If branch[i] is null, it respent there isn't a son node at branch[i]
        // Otherwish it's a son node and the corresponding bit in is_node_bmp respent
        // whether it's a CNode.
        int is_cnode_bmp;
        BasicNode *branch[1 << kBRANCH_WIDTH];

        CNode() {
            is_cnode_bmp = 0;
            memset(branch, 0, sizeof(branch));
            node_type = CNODE;
        }

        CNode(SNode *s1, SNode *s2, int lev) {
            is_cnode_bmp = 0;
            memset(branch, 0, sizeof(branch));
            node_type = CNODE;
            int pos1 = get_pos(s1->key, lev);
            int pos2 = get_pos(s2->key, lev);
            if (pos1 != pos2) {
                branch[pos1] = s1;
                branch[pos2] = s2;
            } else {
                INode *in = new INode();
                update(pos1, true, in);
                in->main = new CNode(s1, s2, lev+1);
            }
        }

        void update(int pos, bool is_cnode, BasicNode *n) {
            branch[pos] = n;
            is_cnode_bmp = is_cnode_bmp & (~(1 << pos)) | int(is_cnode) << pos;
        }
    };

    struct INode: BasicNode {
        BasicNode *main;

        INode(BasicNode *m = NULL) {
            main = m;
            node_type = INODE;
        }

        bool cas_main(BasicNode *om, BasicNode *m) {
            return __sync_bool_compare_and_swap(&main, om, m);
        }
    };


    INode *root;

    static
    int get_pos(int key, int lev) {
        key >>= lev * kBRANCH_WIDTH;
        return key & ((1 << kBRANCH_WIDTH) - 1);
    }

    bool to_contracted(CNode *p, SNode *&tsn) {
        if (p->is_cnode_bmp) return false;
        bool ok = false;
        for (int i = 0; i < (1 << kBRANCH_WIDTH); i++) {
            if (p->branch[i]) {
                if (ok) {
                    return false;
                } else {
                    ok = true;
                    *tsn = *((SNode *) p->branch[i]);
                }
            }
        }
        tsn->entomb();
        return true;
    }

    void clear(INode *in, int lev) {
        // printf("clearing\n");
        CNode *p = (CNode *) in->main;
        if (p->node_type != CNODE) {
            // The INode in has changed!
            return;
        }

        CNode *ncn = new CNode();
        *ncn = *p;
        bool changed = false;
        for (int i = 0; i < (1 << kBRANCH_WIDTH); i++) {
            if (!p->branch[i]) continue;

            bool is_cnode = p->is_cnode_bmp & (1 << i);
            if (is_cnode) {
                INode *subi = (INode *) p->branch[i];
                if (subi->main->node_type == TNODE) {
                    SNode *sn = (SNode *) subi->main;
                    sn->resurrect();
                    ncn->update(i, false, sn);
                    changed = true;
                }
            }
        }

        if (!changed) return;
        BasicNode *nmain = ncn;
        SNode *tsn = new SNode();
        if (to_contracted(ncn, tsn)) {
            nmain = tsn;
        }
        in->cas_main(p, nmain);
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
    for (int i = 0; i < 1e6; i++) {
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

#ifdef MEASURE_COST
                ct.Find(n);
#else
                // printf("Finding %d\n", n);
                assert((s.find(n) != s.end()) == ct.Find(n));
#endif
                break;
            }
            case 1: {
                int n = rand();
                if (l.size() and rand() % 10 == 0) {
                    n = l[rand() % l.size()];
                }

#ifdef MEASURE_COST
                ct.Insert(n);
#else
                // printf("Inserting %d\n", n);
                if (s.find(n) != s.end()) {
                    assert(ct.Insert(n) == false);
                } else {
                    s.insert(n);
                    assert(ct.Insert(n));
                }
#endif
                break;
            }
            case 2: {
                int n = rand();
                if (l.size() and rand() % 7) {
                    n = l[rand() % l.size()];
                }

#ifdef MEASURE_COST
                ct.Delete(n);
#else
                if (s.find(n) != s.end()) {
                    s.erase(n);
                    assert(ct.Delete(n));
                } else {
                    assert(!ct.Delete(n));
                }
#endif
                break;
            }
        }
    }

    printf("Finshed batch test!\n");
}

void make_data(vector<pair<int, int> > &ops, int op_cnt) {
    vector<int> l;
    for (int i = 0; i < op_cnt; i++) {
        // 0 is find, 1 is insert, 2 is delet
        int op = rand() % 2;
        if (rand() % 100 and l.size()) op = 2;
        int oprand = 0;

        if (op == 0) {
            if (rand()%3 and l.size()) {
                oprand = l[rand() % l.size()];
            } else {
                oprand = rand();
            }
        } else if (op == 1) {
            oprand = rand();
        } else {
            oprand = l[rand() % l.size()];
        }

        ops.push_back(make_pair(op, oprand));
    }
}

int Pos;
volatile bool running = false;
CTrie ct;
vector<pair<int, int> > ops;

void run() {
    int p = 0;
    int sz = ops.size();

    while (!running);

    while (true) {
        p = Pos;
        if (p >= sz) break;

        if (!__sync_bool_compare_and_swap(&Pos, p, p+1)) {
            continue;
        }

        int op = ops[p].first, oprand = ops[p].second;
        //printf("op = %d\n", op);
        switch (op) {
            case 0:
                ct.Find(oprand);
                break;
            case 1:
                ct.Insert(oprand);
                break;
            case 2:
                ct.Delete(oprand);
                break;
        }
    }
}

void cost_measure() {
    int op_cnt = 0, th_cnt = 0;
    scanf("%d %d", &op_cnt, &th_cnt);
    make_data(ops, op_cnt);

    thread ths[10];
    printf("th_cnt = %d\n", th_cnt);
    for (int i = 0; i < th_cnt; i++) {
        ths[i] = thread(run);
    }

    clock_t s, t;
    running = true;
    s = clock();
    printf("running!\n");
    for (int i = 0; i < th_cnt; i++) {
        ths[i].join();
    }
    t = clock();

    double ms = (t - s) * 1.0 / CLOCKS_PER_SEC;
    printf("%.5lf\n", ms);
}

int main() {
    //basic_test();
    //test_insfind();
    cost_measure();
    return 0;
}
