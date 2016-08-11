#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <ctime>

#include "hashtable.h"
HashTable h;

struct Operation {
    int op;
    int a, b;
};

void *run_test(std::string file_name) {
    h.ThreadInit();
    clock_t start_t, end_t;
    int op_num;
    std::vector<Operation> ops;
    std::ifstream td;

    td.open(file_name);
    td >> op_num;
    for (int i = 0; i < op_num; i++) {
        char op; int a, b;
        td >> op >> a >> b;
        ops.push_back((Operation) {op, a, b});
    }
    td.close();

    std::cout << "Running test with " << file_name << "." << std::endl;

    start_t = clock();
    for (int i = 0; i < ops.size(); i++) {
        char op = ops[i].op;
        int a = ops[i].a, b = ops[i].b;
        switch (op) {
            case 'P':
                h.Put(a, b);
                break;
            case 'Q': {
                int res = h.Get(a);
                if (res != b) {
                    std::cerr << "Occur error at " << i + 1 << "th operation."
                       << "The expection is " << b << ", but got "
                       << res << " instead." << std::endl;
                    exit(-1);
                }
                break;
            }
            case 'A':
                h.AtomicAdd(a, b);
                break;
            default:
                std::cerr << "Unrecognized operation type." << std::endl;
                exit(-1);
                break;
        }
    }
    end_t = clock();

    double duration = (end_t - start_t) / (CLOCKS_PER_SEC / 1000);

    std::cout << "Finished test with " << file_name << " in " << duration << "ms" << std::endl;
    h.ThreadRelease();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Incorrect number of arguments!" << std::endl;
        exit(-1);
    }

    // run_test(argv[1]);
    
    std::thread ths[100];
    for (int i = 1; i < argc; i++) {
        ths[i] = std::thread(run_test, std::string(argv[i]));
    }

    for (int i = 1; i < argc; i++) {
        ths[i].join();
    }
    
    return 0;
}
