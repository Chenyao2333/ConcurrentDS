#! /usr/bin/env python3

import os
import subprocess
from random import randint

def gen(op_num, bit_range):
    data = "%s %s\n" % (op_num, bit_range)
    arr = [0 for i in range(bit_range)]
    s = 0
    for i in range(op_num):
        if randint(1, 2) == 1 or not s:
            pos = randint(0, bit_range-1)
            delta = randint(-arr[pos], 10)
            data += "ADD %s %s\n" % (pos, delta)
            s += delta
            arr[pos] += delta
        else:
            val = randint(1, s)
            ind = -1
            sum_ = 0
            for v in arr:
                if v + sum_ < val:
                    sum_ += v
                    ind += 1
                else:
                    break
            data += "QUE %s %s %s\n" % (val, ind, sum_)
    return data

#print(gen(10, 5), end="")

def gen_batch():
    sizes = [(10, 5), (20, 5), (100, 10), (200, 50), (300, 50),
             (500, 100), (1000, 5), (1000, 10), (1000, 300), (2000, 100)]
    for op_num, bit_range in sizes:
        d = gen(op_num, bit_range)
        with open("data/BIT_%s_%s.td" % (op_num, bit_range), "w") as f:
            f.write(d)

def check():
    tds = []
    for f in os.listdir("data"):
        if f.endswith(".td"):
            tds.append(f)
    
    for f in tds:
        f = os.path.join("./data", f)
        print("Running with %s!" % f)
        if subprocess.call(["./BIT_checker", f]):
            print("Occur error with %s!\n" % f)
            return

if __name__ == "__main__":
    #gen_batch()
    check()

