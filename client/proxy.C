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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif


#ifndef _WIN32
#include <cstring>
#include <sstream>

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "util.h"
#include "client_msgs.h"
#include "parse.h"

#include "proxy.h"

// Read the contents of the socket into buf
//
static int proxy_read_reply(int socket, char* buf, int len) {
    int i, n;
    for (i=0; i<len-1; i++) {
        n = recv(socket, buf+i, 1, 0);
        if (n != 1) break;
    }
    buf[i] = 0;
    return i;
}

// Make sure we send the full contents of each message.  Messages
// are relatively short, so it's safe
// 
/*static int proxy_atomic_send(int socket, char *buf, int size) {
    int ret, len;
    
    ret = 0;
    while (size > 0) {
        len = send(socket, buf+ret, size, 0);
        if ( len == -1 && errno != ) fatal("atomic_out() failed to send(), %d\n", socks_errno());
        ret += len;
        size -= len;
    }
    return ret;
}
*/

void print_buf( char *buf, int n ) {
    for (int i=0;i<n;i++) printf( "%d ",buf[i] );
    printf( "\n" );
}


PROXY::PROXY() {
    strcpy(proxy_data,"");
    proxy_state = PROXY_STATE_CONNECTING;
    proxy_retval = 0;
}

void PROXY::init(char *dst_host, int _port) {
    strcpy(dest_serv_name, dst_host);
    dest_serv_port = _port;
    proxy_state = PROXY_STATE_CONNECTING;
}

PROXY::~PROXY() {
}

int PROXY::set_proxy(PROXY_INFO *new_pi) {
    pi.use_http_proxy = new_pi->use_http_proxy;
    strcpy(pi.http_user_name, new_pi->http_user_name);
    strcpy(pi.http_user_passwd, new_pi->http_user_passwd);
    strcpy(pi.http_server_name, new_pi->http_server_name);
    pi.http_server_port = new_pi->http_server_port;
    pi.use_http_auth = new_pi->use_http_auth;

    pi.use_socks_proxy = new_pi->use_socks_proxy;
    strcpy(pi.socks5_user_name, new_pi->socks5_user_name);
    strcpy(pi.socks5_user_passwd, new_pi->socks5_user_passwd);
    strcpy(pi.socks_server_name, new_pi->socks_server_name);
    pi.socks_server_port = new_pi->socks_server_port;
    pi.socks_version = new_pi->socks_version;

    return 0;
}

char *PROXY::get_proxy_server_name(char *regular_server) {
    if (pi.use_socks_proxy) return pi.socks_server_name;
    else if (pi.use_http_proxy) return pi.http_server_name;
    else return regular_server;
}

int PROXY::get_proxy_port(int regular_port) {
    if (pi.use_socks_proxy) return pi.socks_server_port;
    else if (pi.use_http_proxy) return pi.http_server_port;
    else return regular_port;
}

int PROXY::proxy_failed(int failure_code) {
    proxy_state = PROXY_STATE_DONE;
    proxy_retval = failure_code;

    return 0;
}

// Initialize proxy_data with a socks4 or socks5 connection request
// see http://www.socks.nec.com/rfc/rfc1928.txt
// see http://www.socks.nec.com/protocol/socks4.protocol
// TODO: add support for GSSAPI authentication, IPv6 addresses, name -> IP resolution
// One current problem is that the client may not fully read/write messages
// to the server if buffers are full.  proxy_atomic_send is an attempt to
// compensate, it should be fully implemented and tested if this becomes a
// significant issue.  I'm not too worried though, since the messages are
// always very small, and unlikely to cause space concerns (Eric Heien 3/26/04)

// Check available methods on the socks server
//
int PROXY::socks_prepare_method_req(char *buf) {
    int nbytes = 0;
    char *marker = buf;

    if (pi.socks_version != SOCKS_VERSION_5) return ERR_SOCKS_UNKNOWN_FAILURE;
    
    nbytes = 3;
    *marker++ = SOCKS_VERSION_5;
    if (*pi.socks5_user_name && *pi.socks5_user_passwd) {
        *marker++ = 2; // 2 possible methods
        *marker++ = SOCKS_AUTH_USER_PASS; // user/pass
        nbytes++;
    } else {
        *marker++ = 1; // 1 possible method:
    }
    *marker++ = SOCKS_AUTH_NONE_NEEDED; // no authentication

    return nbytes;
}

int PROXY::socks_prepare_user_pass(char *buf) {
    int nbytes;
    char *marker = buf;

    if (pi.socks_version != SOCKS_VERSION_5) return ERR_SOCKS_UNKNOWN_FAILURE;

    // Send user and password
    *marker++ = SOCKS5_USER_SUBNEG_VERSION_1;
    *marker++ = strlen(pi.socks5_user_name);
    strncpy(marker, pi.socks5_user_name, strlen(pi.socks5_user_name));
    marker += strlen(pi.socks5_user_name);
    *marker++ = strlen(pi.socks5_user_passwd);
    strncpy(marker, pi.socks5_user_passwd, strlen(pi.socks5_user_passwd));
    nbytes = 1+1+strlen(pi.socks5_user_name)+1+strlen(pi.socks5_user_passwd);

    return nbytes;
}

int PROXY::socks_prepare_connect_req(char *buf, int ns_port, int ip_addr, char *domain_name) {
    int nbytes;
    char *marker = buf, *p;

    if (pi.socks_version == SOCKS_VERSION_4) {
        *marker++ = SOCKS_VERSION_4;
        *marker++ = 1;          // Request connection
        p = (char*)&ns_port;    // to this port
        for (int i=0;i<2;i++) *marker++ = *p++;
        p = (char*)&ip_addr;    // at this IP address
        for (int i=0;i<4;i++) *marker++ = *p++;
        strncpy(marker, pi.socks5_user_name, strlen(pi.socks5_user_name));
        nbytes = 1+1+2+4+strlen(pi.socks5_user_name);
    } else if (pi.socks_version == SOCKS_VERSION_5) {
        if (strlen(domain_name) > 255) return ERR_SOCKS_UNKNOWN_FAILURE;

        *marker++ = SOCKS_VERSION_5;
        *marker++ = 1;  // request connection
        *marker++ = 0;  // reserved
        nbytes = 3;

        if (strlen(domain_name) > 0) {
            nbytes += 2+strlen(domain_name);
            *marker++ = SOCKS5_ADDR_TYPE_DOMAIN_NAME;
            *marker++ = strlen(domain_name);
            p = domain_name;
            for (unsigned int i=0;i<strlen(domain_name);i++) *marker++ = *p++;
        } else {
            nbytes += 1+4;
            *marker++ = SOCKS5_ADDR_TYPE_IPV4;
            p = (char*)&ip_addr;
            for (int i=0;i<4;i++) *marker++ = *p++;
        }
        p = (char*)&ns_port;
        for (int i=0;i<2;i++) *marker++ = *p++;
        nbytes += 2;
    } else {
        return ERR_SOCKS_UNKNOWN_FAILURE;
    }

    return nbytes;
}

int PROXY::socks_parse_method_ack(char *buf) {
    // This parsing only makes sense in SOCKS version 5
    if (pi.socks_version != SOCKS_VERSION_5) return ERR_SOCKS_UNKNOWN_FAILURE;

    // The buffer should start with the version 5 identifier
    if (buf[0] != SOCKS_VERSION_5) return ERR_SOCKS_UNKNOWN_SERVER_VERSION;

    // Depending on the method, move to a different state or return an error
    switch (buf[1]) {
        case SOCKS_AUTH_NONE_NEEDED:
            proxy_state = PROXY_STATE_SOCKS_SEND_CONNECT_REQ;
            break;
        case SOCKS_AUTH_USER_PASS:
            proxy_state = PROXY_STATE_SOCKS_SEND_USER_PASS;
            break;
        case SOCKS_AUTH_GSSAPI:
        case SOCKS_AUTH_NONE_ACCEPTABLE:
        default:
            return ERR_SOCKS_UNSUPPORTED;
    }

    return 0;
}

int PROXY::socks_parse_user_pass_ack(char *buf) {
    // This parsing only makes sense in SOCKS version 5
    if (pi.socks_version != SOCKS_VERSION_5) return ERR_SOCKS_UNKNOWN_FAILURE;

    // The buffer should start with the subnegotiation version identifier
    if (buf[0] != SOCKS5_USER_SUBNEG_VERSION_1) return ERR_SOCKS_UNKNOWN_SERVER_VERSION;

    // 0 indicates success, otherwise we assume a bad user/password
    if (buf[1] != 0) return ERR_SOCKS_BAD_USER_PASS;

    proxy_state = PROXY_STATE_SOCKS_SEND_CONNECT_REQ;

    return 0;
}

int PROXY::socks_parse_connect_ack(char *buf) {
    switch (pi.socks_version) {
        case SOCKS_VERSION_4:
            if (buf[0] != 0) return ERR_SOCKS_UNKNOWN_FAILURE;

            switch (buf[1]) {
                case SOCKS4_REQ_GRANTED:
                    proxy_state = PROXY_STATE_DONE;
                    proxy_retval = 0;
                    break;
                case SOCKS4_REQ_REJECTED:
	                return ERR_SOCKS_REQUEST_FAILED;
                case SOCKS4_REQ_NO_IDENTD:
	                return ERR_SOCKS_REQUEST_FAILED;
                case SOCKS4_REQ_BAD_USER_PASS:
	                return ERR_SOCKS_BAD_USER_PASS;
                default:
                    return ERR_SOCKS_UNKNOWN_FAILURE;
            }
            break;
        case SOCKS_VERSION_5:
            if (buf[0] != SOCKS_VERSION_5) return ERR_SOCKS_UNKNOWN_FAILURE;
            switch (buf[1]) {
                case SOCKS5_CONNECTION_SUCCEEDED:
                    proxy_state = PROXY_STATE_DONE;
                    proxy_retval = 0;
                    break;
                case SOCKS5_ERR_SERVER_FAILURE:
                    return ERR_SOCKS_UNKNOWN_FAILURE;
                case SOCKS5_ERR_NOT_ALLOWED:
                    return ERR_SOCKS_REQUEST_FAILED;
                case SOCKS5_ERR_NETWORK_UNREACHABLE:
                    return ERR_SOCKS_CANT_REACH_HOST;
                case SOCKS5_ERR_HOST_UNREACHABLE:
                    return ERR_SOCKS_CANT_REACH_HOST;
                case SOCKS5_ERR_CONNECTION_REFUSED:
                    return ERR_SOCKS_CONN_REFUSED;
                case SOCKS5_ERR_TTL_EXPIRED:
                    return ERR_SOCKS_UNKNOWN_FAILURE;
                case SOCKS5_ERR_COMMAND_UNSUPPORTED:
                    return ERR_SOCKS_UNKNOWN_FAILURE;
                case SOCKS5_ERR_ADDR_TYPE_UNSUPPORTED:
                    return ERR_SOCKS_UNKNOWN_FAILURE;
                default:
                    return ERR_SOCKS_UNKNOWN_FAILURE;
            }
            break;
        default:
            return ERR_SOCKS_UNKNOWN_FAILURE;
    }

    return 0;
}

bool PROXY::proxy_negotiated() {
    return (proxy_state == PROXY_STATE_DONE);
}

bool PROXY::proxy_poll() {
    int n, retval, ip_addr, nbytes;
    bool action = false;
    char buf[256];

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_PROXY);

    switch(proxy_state) {
        case PROXY_STATE_CONNECTING:
            if (pi.use_socks_proxy) {
                if (pi.socks_version == SOCKS_VERSION_4) {
                    scope_messages.printf("PROXY::proxy_poll(): attempting SOCKS4 connect\n");
                    proxy_state = PROXY_STATE_SOCKS_SEND_CONNECT_REQ;
                } else if (pi.socks_version == SOCKS_VERSION_5) {
                    scope_messages.printf("PROXY::proxy_poll(): attempting SOCKS5 connect\n");
                    proxy_state = PROXY_STATE_SOCKS_SEND_METHOD_REQ;
                } else {
                    scope_messages.printf("PROXY::proxy_poll(): SOCKS proxy ignored - unknown version: %d\n", pi.socks_version);
                    proxy_state = PROXY_STATE_DONE;
                }
            } else {
                scope_messages.printf("PROXY::proxy_poll(): SOCKS proxy ignored - user pref\n");
                proxy_state = PROXY_STATE_DONE;
            }
            
            action = true;
            break;
        case PROXY_STATE_SOCKS_SEND_METHOD_REQ:
            nbytes = socks_prepare_method_req(proxy_data);
            scope_messages.printf("PROXY::proxy_poll(): sending method request\n");
            if (nbytes <= 0) {
                proxy_failed(ERR_SOCKS_UNKNOWN_FAILURE);
            } else {
                n = send(socket, proxy_data, nbytes, 0);

                if (n != nbytes) proxy_failed(ERR_SOCKS_UNKNOWN_FAILURE);
                else proxy_state = PROXY_STATE_SOCKS_PARSE_METHOD_ACK;
            }
            action = true;
            break;
        case PROXY_STATE_SOCKS_PARSE_METHOD_ACK:
            if (io_ready) {
                nbytes = proxy_read_reply(socket, buf, 256);
                if (nbytes == 0) break;
                scope_messages.printf("PROXY::proxy_poll(): parsing method request ack\n");
                retval = socks_parse_method_ack(buf);
                if (retval) proxy_failed (retval);
                action = true;
            }
            break;
        case PROXY_STATE_SOCKS_SEND_USER_PASS:
            scope_messages.printf("PROXY::proxy_poll(): sending user/pass\n");
            nbytes = socks_prepare_user_pass(proxy_data);
            if (nbytes <= 0) {
                proxy_failed(ERR_SOCKS_UNKNOWN_FAILURE);
            } else {
                n = send(socket, proxy_data, nbytes, 0);
                if (n != nbytes) proxy_failed(ERR_SOCKS_UNKNOWN_FAILURE);
                else proxy_state = PROXY_STATE_SOCKS_PARSE_USER_PASS_ACK;
            }
            action = true;
            break;
        case PROXY_STATE_SOCKS_PARSE_USER_PASS_ACK:
            if (io_ready) {
                action = true;
                nbytes = proxy_read_reply(socket, buf, 256);
                if (nbytes == 0) break;
                scope_messages.printf("PROXY::proxy_poll(): parsing user/pass ack\n");
                retval = socks_parse_user_pass_ack(buf);
                if (retval) proxy_failed(retval);
            }
            break;
        case PROXY_STATE_SOCKS_SEND_CONNECT_REQ:
            retval = get_ip_addr(ip_addr);
            if (retval) {
                proxy_failed(retval);
                break;
            }
            scope_messages.printf("PROXY::proxy_poll(): sending connect request\n");
            nbytes = socks_prepare_connect_req(proxy_data, htons(dest_serv_port), ip_addr, dest_serv_name);
            if (nbytes <= 0) {
                proxy_failed(ERR_SOCKS_UNKNOWN_FAILURE);
            } else {
                n = send(socket, proxy_data, nbytes, 0);
                if (n != nbytes) proxy_failed(ERR_SOCKS_UNKNOWN_FAILURE);
                else proxy_state = PROXY_STATE_SOCKS_PARSE_CONNECT_ACK;
            }
            action = true;
            break;
        case PROXY_STATE_SOCKS_PARSE_CONNECT_ACK:
            if (io_ready) {
                action = true;
                nbytes = proxy_read_reply(socket, buf, 256);
                if (nbytes == 0) break;
                scope_messages.printf("PROXY::proxy_poll(): parsing connect ack\n");
                retval = socks_parse_connect_ack(buf);
                if (retval) {
                    proxy_failed(retval);
                }
            }
            break;
    }
    return action;
}


const char *BOINC_RCSID_ff688e91e4 = "$Id$";
