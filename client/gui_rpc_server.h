class GUI_RPC_CONN {
public:
    int sock;
    FILE* fout;
    GUI_RPC_CONN(int);
    ~GUI_RPC_CONN();
    int handle_rpc();
};

class GUI_RPC_CONN_SET {
    vector<GUI_RPC_CONN*> gui_rpcs;
    int insert(GUI_RPC_CONN*);
    int lsock;
public:
    bool poll();
    void init(char*);
};

