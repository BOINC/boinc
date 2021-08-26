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

#include <sstream>
#include <sockpp/socket.h>
#include <sockpp/tcp_connector.h>
#ifdef _WIN32
    #include <Windows.h>
#else
    #include <sockpp/unix_connector.h>
#endif


void mySleep(int seconds) {
#ifdef _WIN32
    Sleep(seconds * 1000);
#else
    sleep(seconds);
#endif // _WIN32
}

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

    bool isDebug = false;
    sockpp::socket_initializer sockInit;
    sockpp::tcp_connector tcp_conn;
#if !_WIN32
    sockpp::unix_connector unix_conn;
#endif
    sockpp::connector* conn = NULL;
    int16_t port = 31416;
    std::string android_address = "edu_berkeley_boinc_client_socket";
    std::string unix_address = "boinc_socket";
    std::string tcp_address = "localhost";
    bool isConn = false;
    std::stringstream stream;
    const int sizeBuffer = 1024;
    char buf[sizeBuffer];
    int suffix_file = 0;
    while (true)
    {
        int attemped_connections = 0;
#if _WIN32
        isConn = isConnected(tcp_conn);
#else
        isConn = isConnected(tcp_conn, unix_conn);
#endif
        while (!isConn) {
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
            isConn = isConnected(tcp_conn, unix_conn);
            if (isConn) {
                conn = getConnected(tcp_conn, unix_conn);
            }
#else
            isConn = isConnected(tcp_conn);
            if (isConn) {
                conn = getConnected(tcp_conn);
            }
#endif

            attemped_connections++;
            printf("attemped connections: %d\n\n", attemped_connections);
            mySleep(1);
        }

        std::string request = "<boinc_gui_rpc_request><get_state/></boinc_gui_rpc_request>\3";
        if (isDebug) {
            printf("before NULL == conn: %ld\n", conn);
        }
        if(NULL == conn) {
#if !_WIN32
            unix_conn.close();
#endif
            tcp_conn.close();
            continue;
        }
        if (isDebug) {
            printf("before write: \n");
        }
        int res = conn->write(request);

        if (isDebug) {
            printf("After write: %d\n", res);
        }
        if (res == -1) {
            conn->close();
#if !_WIN32
            unix_conn.close();
#endif
            tcp_conn.close();
            continue;
        }
        
        printf("request bytes: %ld\n", request.size());
        printf("write bytes: %d\n", res);

        if (isDebug) {
            printf("After write, before read: \n");
        }

        ssize_t n = sizeBuffer;
        stream.str("");
        while (n == sizeBuffer) {
            n = conn->read(buf, sizeof(buf));
            if (isDebug) {
                printf("result:\n");
                printf("%s\n", buf);
            }
            stream << buf;
;
            if (n < sizeBuffer) {
                printf("buf: %s\n", buf);
            }
        }

        printf("read bytes: %d\n", stream.str().size());
        mySleep(1);
    }
}
