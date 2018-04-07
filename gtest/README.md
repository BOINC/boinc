This is the start gtest example for unit tests.  To run, you should only need to run these commands.

Setup:
You will need to have googletest downloaded and compiled on your local machine first.  The Makefile expects it to be in the same directory level as boinc.  (example:  /home/user/boinc and /home/user/googletest)

    git clone https://github.com/google/googletest.git
    cd googletest
    git pull
    cd googletest/make
    make

The next commands are done within the boinc/gtest folder.

To compile the code. 
make -f Makefile.gtest

To run the tests.
make -f Makefile.gtest test

To cleanup the old files. 
make -f Makefile.gtest clean
