#! /usr/bin/env python3

import argparse
import os
from random import randint

INT_MIN = -2147483648
INT_MAX =  2147483647

class RandomQueue(object):
    def __init__(self):
        self.q = []

    def push(self, key):
        self.q.append(key)
        i = randint(0, len(self.q) - 1)
        self.q[i], self.q[-1] = self.q[-1], self.q[i]

    def pop(self):
        return self.q.pop()

class Generator(object):
    def __init__(self):
        self.keys = RandomQueue()
        self.cur_map = {}

    def __len__(self):
        return len(self.cur_map)

    def gen_set(self):
        key = randint(INT_MIN, INT_MAX)
        val = randint(INT_MIN, INT_MAX)

        if key not in self.cur_map:
            self.keys.push(key)
        self.cur_map[key] = val

        return (key, val)

    def gen_query(self):
        key = self.keys.pop()
        self.keys.push(key)

        return (key, self.cur_map[key])

    def gen_del(self):
        key = self.keys.pop()
        del self.cur_map[key]
        
        return key

class V2Generator(Generator):
    def __init__(self):
        super(V2Generator, self).__init__()

    def gen_set(self):
        key = randint(INT_MIN, INT_MAX)
        val = randint(1, INT_MAX)

        if key not in self.cur_map:
            self.keys.push(key)
        self.cur_map[key] = val

        return (key, val)

    def gen_add(self):
        key = self.keys.pop()

        #print("-self.cur_map[%d] = %d" % (key, -self.cur_map[key]))
        delta = randint(-self.cur_map[key], 10)

        self.cur_map[key] += delta
        if (self.cur_map[key] is 0):
            del self.cur_map[key]
        else:
            self.keys.push(key)

        return key, delta

class AscGenerator(Generator):
    def __init__(self, tot):
        super(AscGenerator, self).__init__()

        # sort keys descending, so that it's ascending when pop from back.
        self.desc_keys = [randint(INT_MIN, INT_MAX) for i in range(tot)]
        self.desc_keys.sort(reverse=True)

    def gen_set(self):
        key = self.desc_keys.pop()
        val = randint(INT_MIN, INT_MAX)

        self.keys.push(key)
        self.cur_map[key] = val

        return (key, val)


def generate(num, data_file, data_type):
    if data_type != "v2":
        print("Unrecorigend data_type!")
        os.exit(-1)

    gen = V2Generator()

    with open(data_file, "w") as f:
        f.write("%d\n" % num)
        for i in range(num):
            while True:
                op_type = randint(1, 3)
                if op_type == 1 and len(gen):      # query
                    key, val = gen.gen_query()
                    f.write("Q %d %d\n" % (key, val))
                    break
                elif op_type == 2:                # put
                    key, val = gen.gen_set()
                    f.write("P %d %d\n" % (key, val))
                    break
                elif op_type == 3 and len(gen) and False:    # add # !!! abandon temporality
                    key, delta = gen.gen_add()
                    f.write("A %d %d\n" % (key, delta))
                    break

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("operation_num", type=int)
    parser.add_argument("-f", dest="data_file", default="data.in") 
    parser.add_argument("-t", dest="data_type", default="v2")
    args = parser.parse_args()

    print("Generating %s (%s)operations into %s!" % (args.operation_num, args.data_type, args.data_file))

    generate(args.operation_num, args.data_file, args.data_type)
