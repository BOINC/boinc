// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef _PROXY_
#define _PROXY_

#include "net_xfer.h"
#include "proxy_info.h"

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
