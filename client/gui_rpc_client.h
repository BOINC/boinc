#include <stdio.h>

class RPC_CLIENT {
    int sock;
    FILE* fin;
    FILE* fout;
public:
    ~RPC_CLIENT();
    int init(char*);
    int get_state();
};

