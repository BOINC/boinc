#!/bin/bash

cd gtest && make -f Makefile.gtest && make -f Makefile.gtest test && cd ..

cd lib && gcov -l *.o && cd ..
cd sched && gcov -l *.o && cd ..

# disabled because codecov.io is not connected to github.com/BOINC/boinc
#bash <(curl -s https://codecov.io/bash) -X gcov -X coveragepy -s lib/ -s sched/
