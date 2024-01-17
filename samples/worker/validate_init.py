#! /usr/bin/env python3

# check that a file is uppercase

import sys

def is_uc(path):
    with open(path) as f:
        data = f.read()
    return data == data.upper()

exit(0 if is_uc(sys.argv[1]) else 1)
