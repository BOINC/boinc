#ifndef _PROXY_INFO_
#define _PROXY_INFO_

#include "miofile.h"

struct PROXY_INFO {
    bool use_http_proxy;
    bool use_socks_proxy;
	bool use_http_auth;
    int socks_version;
    char socks_server_name[256];
    char http_server_name[256];
    int socks_server_port;
    int http_server_port;
    char http_user_name[256];
    char http_user_passwd[256];
    char socks5_user_name[256];
    char socks5_user_passwd[256];

    int parse(MIOFILE&);
    int write(MIOFILE&);
    void clear();
};

#endif