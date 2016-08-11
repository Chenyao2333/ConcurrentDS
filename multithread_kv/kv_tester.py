#! /usr/bin/env python3

import argparse
import sys
import subprocess
import os

from data_generator import generate


def test():
    sz = [10, 20, 100, 10000, 100000, 1000000]
    
    for s in sz:
        file_name = "data/KV_%d.td" % s

        if not os.path.isfile(file_name): 
            generate(s, file_name, "v2")

        #print("Testing with %s" % file_name)
        retc = subprocess.call(["./kvtester", file_name])
        if retc:
            print ("Wrong return code! ", retc)
            sys.exit(-1)

if __name__ == "__main__":
    #parser = argparse.ArgumentParser()
    #parser.add_argument("-d", 
    test()
