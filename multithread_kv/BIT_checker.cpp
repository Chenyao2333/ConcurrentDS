#include <iostream>
#include <string>

#include "hashtable.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect number of arguments" << std::endl;
    }

    freopen(argv[1], "r", stdin);
    
    int op_num, max_range;
    std::cin >> op_num >> max_range;

    BinaryIndexTree<int> bit(max_range);

    for (int i = 1; i <= op_num; i++) {
        std::string op_type;
        std::cin >> op_type;

        if (op_type == "ADD") {
            int pos, delta;
            std::cin >> pos >> delta;
            bit.Add(pos, delta);
        } else {
            int val, pos, sum;
            std::cin >> val >> pos >> sum;
            
            auto res = bit.last_less(val);
            if (res.first != pos or res.second != sum) {
                std::cout << "Occur error at " << i << "th operation in " << argv[1] << ". The expection is " << pos << " " << sum << ", but found " << res.first << " " << res.second << "." << std::endl;
                return -1;
            }
        }
    }

    return 0;
}
