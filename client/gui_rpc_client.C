#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "parse.h"
#include "error_numbers.h"
#include "gui_rpc_client.h"

int RPC_CLIENT::init(char* path) {
    int sock, retval;
    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    retval = connect(sock, (const sockaddr*)(&addr), sizeof(addr));
    if (retval) {
        perror("connect");
        exit(1);
    }
    fin = fdopen(dup(sock), "r");
    fout = fdopen(sock, "w");
}

RPC_CLIENT::~RPC_CLIENT() {
    fclose(fin);
    fclose(fout);
}

int RPC_CLIENT::get_state() {
    char buf[256];
    int retval;

    fprintf(fout, "<get_state/>\n");
    fflush(fout);
    while (fgets(buf, 256, fin)) {
        printf(buf);
        if (match_tag(buf, "</client_state>")) break;
    }
    return 0;
}
