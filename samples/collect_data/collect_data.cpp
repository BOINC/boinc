// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2021 University of California
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
#include <fstream>
#include <iostream>
#include <string>
#include <sockpp/socket.h>
#include <sockpp/tcp_connector.h>
#ifdef _WIN32
    #include <Windows.h>
#else
    #include <sockpp/unix_connector.h>
#endif

void my_sleep(int seconds) {
#ifdef _WIN32
    Sleep(seconds * 1000);
#else
    sleep(seconds);
#endif // _WIN32
}

sockpp::connector* get_connected(sockpp::connector* array_connection[]) {
    for (int i = 0; i < 2; ++i) {
        sockpp::connector* conn = array_connection[i];
        if (conn != NULL && conn->is_connected()) {
            return conn;
        }
    }
    return NULL;
}

bool is_connected(sockpp::connector* array_connection[]) {
    for (int i = 0; i < 2; ++i) {
        sockpp::connector* conn = array_connection[i];
        if (conn != NULL && conn->is_connected()) {
            return true;
        }
    }
    return false;
}

int main(int, char**) {
    //boinc_init();

    bool isDebug = false;
    sockpp::socket_initializer sockInit;
    sockpp::tcp_connector tcp_conn;
    sockpp::connector* array_connection[] = { &tcp_conn, NULL };
#if !_WIN32
    sockpp::unix_connector unix_conn;
    array_connection[1] = &unix_conn;
#endif
    sockpp::connector* conn = NULL;
    const int16_t port = 31416;
    const std::string android_address = "edu_berkeley_boinc_client_socket";
    const std::string unix_address = "boinc_socket";
    const std::string tcp_address = "localhost";
    const std::string prefix_file_name = "boinc";
    const int buffer_size = 1024;
    std::stringstream stream;
    char buf[buffer_size];
    int suffix_file_idx = 0;
    while (true)
    {
        int attemped_connections = 0;
        while (!is_connected(array_connection)) {
            attemped_connections++;
            std::cout << "attemped connections: " << attemped_connections << "\n";
            if (!tcp_conn.connect(sockpp::inet_address(tcp_address, port))) {
                std::cout << "tcp connection fail:\n";
                std::cout << tcp_conn.last_error_str() << "\n";
                tcp_conn.clear();
            }
#if! _WIN32
            if (!isConnected(array_connection) && !unix_conn.connect(sockpp::unix_address(unix_address))) {
                std::cout << "unix connection fail:\n";
                std::cout << unix_conn.last_error_str() << "\n";
                unix_conn.clear();                
            }

            if (!isConnected(array_connection) && !unix_conn.connect(sockpp::unix_address(android_address))) {
                std::cout << "android unix connection fail:\n";
                std::cout << unix_conn.last_error_str() << "\n";
                unix_conn.clear();                
            }
#endif
            if (is_connected(array_connection)) {
                conn = get_connected(array_connection);
            }
            std::cout << "\n";
            my_sleep(1);
        }

        std::string request = "<boinc_gui_rpc_request><get_state/></boinc_gui_rpc_request>\3";
        if (isDebug) {
            std::cout << "before NULL == conn: " << conn << "\n";
        }
        if(NULL == conn) {
#if !_WIN32
            unix_conn.close();
#endif
            tcp_conn.close();
            continue;
        }
        if (isDebug) {
            std::cout << "before write: \n";
        }
        int res = conn->write(request);

        if (isDebug) {
            std::cout << "after write: " << res << "\n";            
        }
        if (res == -1) {
            conn->close();
#if !_WIN32
            unix_conn.close();
#endif
            tcp_conn.close();
            continue;
        }
        
        std::cout << "request bytes: " << request.size() << "\n";
        std::cout << "write bytes: " << res << "\n";

        if (isDebug) {
            std::cout << "after write, before read: \n";
        }

        ssize_t n = buffer_size;
        stream.str("");
        while (n == buffer_size) {
            n = conn->read(buf, sizeof(buf));
            if (isDebug) {
                std::cout << "result size: " << n << "\n";
                std::cout << "buf: " << buf << "\n";
            }
            if (n > 0) {
                std::string str(buf, buf + n);
                stream << str;
            }
        }
        std::string str = stream.str();
        str = str.substr(0, str.size() - 1);
        std::cout << "read bytes: " << str.size() << "\n";
        std::ofstream outFile;
        std::stringstream file_name;
        file_name.str("");
        file_name << prefix_file_name << suffix_file_idx << ".txt";
        outFile.open(file_name.str());
        outFile << str;
        std::cout << "write to file: " << prefix_file_name << suffix_file_idx << ".txt\n\n";
        suffix_file_idx++;
        outFile.flush();
        outFile.close();
        my_sleep(1);
    }
}
