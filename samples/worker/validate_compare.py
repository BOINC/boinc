#! /usr/bin/env python3

# check that 2 files are identical

import sys

def read(path):
    with open(path) as f:
        data = f.read()
    return data

exit(0 if read(sys.argv[1])==read(sys.argv[2]) else 1)
