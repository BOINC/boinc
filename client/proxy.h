// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifndef _PROXY_
#define _PROXY_

#include "net_xfer.h"

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

class PROXY : public NET_XFER {
public:
    PROXY();
    ~PROXY();

    PROXY_INFO pi;
    char proxy_data[256];
    int proxy_state;
    int proxy_retval;
    char dest_serv_name[256];
    int dest_serv_port;

    void init(char *dst_host, int port);
    bool proxy_poll();
    bool proxy_negotiated();
    int proxy_failed(int failure_code);
    char *get_proxy_server_name(char *regular_server);
    int get_proxy_port(int regular_port);
    int set_proxy(PROXY_INFO *new_pd);
    int socks_prepare_method_req(char *buf);
    int socks_prepare_user_pass(char *buf);
    int socks_prepare_connect_req(char *buf, int ns_port, int ip_addr, char *domain_name);
    int socks_parse_method_ack(char *buf);
    int socks_parse_user_pass_ack(char *buf);
    int socks_parse_connect_ack(char *buf);
};

void print_buf(char *buf, int n);

#define PROXY_STATE_CONNECTING                0
#define PROXY_STATE_SOCKS_SEND_METHOD_REQ     1
#define PROXY_STATE_SOCKS_PARSE_METHOD_ACK    2
#define PROXY_STATE_SOCKS_SEND_USER_PASS      3
#define PROXY_STATE_SOCKS_PARSE_USER_PASS_ACK 4
#define PROXY_STATE_SOCKS_SEND_CONNECT_REQ    5
#define PROXY_STATE_SOCKS_PARSE_CONNECT_ACK   6
#define PROXY_STATE_DONE                      7

// SOCKS #defines
#define SOCKS_VERSION_4             0x04
#define SOCKS_VERSION_5             0x05

#define SOCKS_AUTH_NONE_NEEDED      0x00
#define SOCKS_AUTH_GSSAPI           0x01
#define SOCKS_AUTH_USER_PASS        0x02
#define SOCKS_AUTH_NONE_ACCEPTABLE  0xFF

#define SOCKS4_REQ_GRANTED          90
#define SOCKS4_REQ_REJECTED         91
#define SOCKS4_REQ_NO_IDENTD        92
#define SOCKS4_REQ_BAD_USER_PASS    93

#define SOCKS5_ADDR_TYPE_IPV4        0x01
#define SOCKS5_ADDR_TYPE_DOMAIN_NAME 0x03
#define SOCKS5_ADDR_TYPE_IPV6        0x04

#define SOCKS5_CONNECTION_SUCCEEDED      0x00
#define SOCKS5_ERR_SERVER_FAILURE        0x01
#define SOCKS5_ERR_NOT_ALLOWED           0x02
#define SOCKS5_ERR_NETWORK_UNREACHABLE   0x03
#define SOCKS5_ERR_HOST_UNREACHABLE      0x04
#define SOCKS5_ERR_CONNECTION_REFUSED    0x05
#define SOCKS5_ERR_TTL_EXPIRED           0x06
#define SOCKS5_ERR_COMMAND_UNSUPPORTED   0x07
#define SOCKS5_ERR_ADDR_TYPE_UNSUPPORTED 0x08

#define SOCKS5_USER_SUBNEG_VERSION_1     1

#endif
