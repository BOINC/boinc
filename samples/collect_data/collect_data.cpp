// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// Application for testing non-CPU-intensive features of BOINC.
// TODO: make it exit after a certain amount of time

//#include "str_util.h"
//#include "util.h"
//#include "boinc_api.h"
#include <sockpp/socket.h>
#include <sockpp/tcp_connector.h>
#if !_WIN32
#include <sockpp/unix_connector.h>
#endif

#if _WIN32
sockpp::connector* getConnected(sockpp::tcp_connector& tcp_conn) {
    if (tcp_conn.is_connected()) {
        return &tcp_conn;
    }
    return NULL;
}
#else
sockpp::connector* getConnected(sockpp::tcp_connector& tcp_conn, sockpp::unix_connector& unix_conn) {
    if (tcp_conn.is_connected()) {
        return &tcp_conn;
    }

    if (unix_conn.is_connected()) {
        return &unix_conn;
    }

    return NULL;
}
#endif

#if _WIN32
bool isConnected(sockpp::tcp_connector& conn) {
    return conn.is_connected();
}
#else
bool isConnected(sockpp::tcp_connector& conn, sockpp::unix_connector& unix_conn) {
    return conn.is_connected() || unix_conn.is_connected();
}
#endif

int main(int, char**) {
    //boinc_init();

    sockpp::tcp_connector tcp_conn;
#if !_WIN32
    sockpp::unix_connector unix_conn;
#endif
    sockpp::connector* conn = NULL;
    int16_t port = 31416;
    std::string android_address = "edu_berkeley_boinc_client_socket";
    std::string unix_address = "boinc_socket";
    std::string tcp_address = "localhost";
    while (true)
    {
        int attemped_connections = 0;
#if _WIN32
        while (!isConnected(tcp_conn)) {
#else
        while(!isConnected(tcp_conn, unix_conn)) {
#endif
            if (!tcp_conn.connect(sockpp::inet_address(tcp_address, port))) {
                printf("tcp connection fail:\n");
                printf("%s\n",tcp_conn.last_error_str().c_str());
                tcp_conn.clear();
            }
#if! _WIN32
            if (!isConnected(tcp_conn, unix_conn) && !unix_conn.connect(sockpp::unix_address(unix_address))) {
                printf("unix connection fail:\n");
                printf("%s\n",unix_conn.last_error_str().c_str());
                unix_conn.clear();                
            }

            if (!isConnected(tcp_conn, unix_conn) && !unix_conn.connect(sockpp::unix_address(android_address))) {
                printf("android unix connection fail:\n");
                printf("%s\n",unix_conn.last_error_str().c_str());
                unix_conn.clear();                
            }

            if (isConnected(tcp_conn, unix_conn)) {
                conn = getConnected(tcp_conn, unix_conn);
            }
#else
            if (isConnected(tcp_conn)) {
                conn = getConnected(tcp_conn);
            }
#endif


            attemped_connections++;
            printf("attemped connections: %d\n\n", attemped_connections);
            //boinc_sleep(2);
        }
        printf("before write: \n");
        char buf[1024];
        std::string request = "<boinc_gui_rpc_request><get_state/></boinc_gui_rpc_request>";
        
        if(NULL == conn) {
#if !_WIN32
            unix_conn.close();
#endif
            tcp_conn.close();
            continue;
        }

        int res = conn->write(request);

        if (res == -1) {
            conn->close();
#if !_WIN32
            unix_conn.close();
#endif
            tcp_conn.close();
            continue;
        }
        
        printf("request size: %ld\n", request.size());
        printf("res: %d\n", res);
        printf("After write, before read: \n");
        ssize_t n = conn->read(buf, sizeof(buf));
        printf("result:\n");
        printf("%s\n", buf);
        //boinc_sleep(2);
    }
}
