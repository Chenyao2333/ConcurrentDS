#include <cstdio>
#include <iostream>
#include <algorithm>

const int kPOOL_SIZE = 2e5 + 10;

struct Node {
    int key, val;

    int height;
    Node *fa, *ls, *rs;

    Node(int key = 0, int val = 0): key(key), val(val) {
        height = 1;
        fa = ls = rs = NULL;
    }

    void update() {
        // leaf
        if (! (ls or rs)) {
            height = 1;
            return;
        }

        val = ls->val + rs->val;

        height = std::max(ls->height, rs->height) + 1;
    }

    int get_factor() {
        // leaf
        if (! (ls or rs)) {
            return 0;
        }

        return ls->height - rs->height;
    }
};

class AVLTree {
public:
    AVLTree() {
        root_holder = new_node();
    }

    void Insert(int key, int val) {
        if (root_holder->ls) {
            root_holder->ls = insert_(root_holder->ls, key, val);
            root_holder->ls->fa = root_holder;
        } else {
            root_holder->ls = new_node();
            *(root_holder->ls) = Node(key, val);
            root_holder->ls->fa = root_holder;
        }
    }

    int Get(int key) {
        if (root_holder->ls) {
            return get_(root_holder->ls, key);
        } else {
            return 0;
        }
    }

    void Delete(int key) {
        if (root_holder->ls) {
            if (root_holder->ls->height == 1) {
                root_holder->ls = NULL;
                // delete ls
            } else {
                root_holder->ls = delete_(root_holder->ls, key);
            }
        }
    }

    int PreSum(int key) {
        int ans = pre_sum_(root_holder->ls, key);
        return ans;
    }

    // Return the first key which its prefix sum  greater than 'val'
    int PreSumGreat(int val) {
        int key = pre_sum_great_(root_holder->ls, val);
        return key;
    }

    int Prec(int key) {
        Node *x = find_(root_holder->ls, key);
        x = prec_(x);
        return x->key;
    }

    int Succ(int key) {
        Node *x = find_(root_holder->ls, key);
        x = succ_(x);
        return x->key;
    }
private:
    Node *root;

    bool is_leaf(Node *x) { return !(x->ls or x->rs); }

    void zig(Node *x) {
        Node *y = x->fa;
        Node *z = y->fa;

        if (z->ls == y) z->ls = x;
        else            z->rs = x;
        x->fa = z;

        y->ls = x->rs; if (x->rs) x->rs->fa = x;

        x->rs = y; y->fa = x;
    }

    void zag(Node *x) {
        Node *y = x->fa;
        Node *z = y->fa;

        if (z->ls == y) z->ls = x;
        else            z->rs = x;
        x->fa = z;

        y->rs = x->ls; if (x->ls) x->ls->fa = y;

        x->ls = y; y->fa = x;
    }

    Node* blance(Node *x) {
        x->update();
        int xfactor = x->get_factor();
        if (xfactor == 2) {
            Node *y = x->ls;
            if (y->get_factor() == -1) {
                y = y->rs;
                zag(y);
            }
            zig(y);
            return y;
        } else if (xfactor == -2) {
            Node *y = x->rs;
            if (y->get_factor() == 1) {
                y = y->ls;
                zig(y);
            }
            zag(y);
        }

        return x;
    }

    int pre_sum_(Node *x, int key) {
        if (!(x->ls or x->rs)) {
            if (key <= x->key) return x->val;
            else return 0;
        }

        if (key <= x->key) {
            return pre_sum_(x->ls, key);
        } else {
            return pre_sum_(x->rs, key) + x->ls->val;
        }
    }

    int pre_sum_great_(Node *x, int val) {
        if (!(x->ls or x->rs)) {
            return x->key;
        }

        if (x->ls->val < val) {
            val -= x->ls->val;
            return pre_sum_great_(x->rs, val);
        } else {
            return pre_sum_great_(x->ls, val);
        }
    }

    Node* insert_(Node* x, int key, int val) {
        if (is_leaf(x)) {
            if (key == x->key) {
                x->val = val;
                return x;
            }
            
            Node *ls = x, *rs = new_node();
            *rs = Node(key, val);
            if (ls->val > rs->val) std::swap(ls, rs);

            x = new_node();
            *x = Node(ls->key, 0);
            x->ls = ls, x->rs = rs;
            x->update();
            
            return x;
        }

        if (key <= x->key) {
            x->ls = insert_(x->ls, key, val);
            x->ls->fa = x;
            return blance(x);
        } else {
            x->rs = insert_(x->rs, key, val);
            x->rs->fa = x;
            return blance(x);
        }
    }

    int get_(Node *x, int key) {
        if (is_leaf(x)) {
            return x->key == key ? x->val : 0;
        }

        if (key <= x->key) {
            return get_(x->ls, key);
        } else {
            return get_(x->rs, key);
        }
    }

    Node* delete_(Node *x, int key) {
        if (!x) {
            return NULL;
        }

        if (key <= x->key) {
            if (x->ls->key == key) {
                x->rs->fa = x->fa;
                // delete x and x->ls
                return x->rs;
            }

            x->ls = delete_(x->ls, key);
            x->ls->fa = x;
            return blance(x);
        } else {
            if (x->rs->key == key) {
                x->ls->fa = x->fa;
                // delete x and x->rs
                return x->ls;
            }

            x->rs = delete_(x->rs, key);
            x->rs->fa = x;
            return blance(x);
        }
    }

    Node *prec_(Node *x) {
        Node *p = x->fa;
        while (x == p->ls) {
            x = p;
            p = x->fa;
        }

        x = p->ls;
        while (x->rs) x = x->rs;
        return x;
    }

    Node *find_(Node *x, int key) {
        if (is_leaf(x)) {
            return x;
        }

        if (key <= x->key) {
            return find_(x->ls, key);
        } else {
            return find_(x->rs, key);
        }
    }

    Node *succ_(Node *x) {
        Node *p = x->fa;
        while (x == p->rs) {
            x = p;
            p = x->fa;
        }

        x = p->rs;
        while (x->ls) x = x->ls;
        return x;
    }


    Node pool[kPOOL_SIZE];
    int node_cnt;

    Node *root_holder;

    Node* new_node() {
        return &pool[node_cnt++];
    }
};

AVLTree t;
int main() {
    freopen("in.txt", "r", stdin);
    int n;
    scanf("%d", &n);
    
    for (int i = 1; i <= n; i++) {
        int op, x; scanf("%d %d", &op, &x);
        printf("op = %d x = %d\n", op, x);
        switch (op) {
            case 1: {
                int val = t.Get(x) + 1;
                printf("get val = %d\n", val);
                t.Insert(x, val);
                break;
            }
            case 2: {
                int val = t.Get(x) - 1;
                if (val == 0) t.Delete(x);
                else          t.Insert(x, val);
                break;
            }
            case 3: {
                int rank = t.PreSum(x);
                printf("%d\n", rank);
                break;
            }
            case 4: {
                int key = t.PreSumGreat(x);
                printf("%d\n", key);
                break;
            }
            case 5: {
                int prec = t.Prec(x);
                printf("%d\n", prec);
                break;
            }
            case 6: {
                int succ = t.Succ(x);
                printf("%d\n", succ);
                break;
            }
        }
    }
    return 0;
}
