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

void mySleep(int seconds) {
#ifdef _WIN32
    Sleep(seconds * 1000);
#else
    sleep(seconds);
#endif // _WIN32
}

sockpp::connector* getConnected(sockpp::connector* array_connection[]) {
    for (int i = 0; i < 2; ++i) {
        sockpp::connector* conn = array_connection[i];
        if (conn != NULL && conn->is_connected()) {
            return conn;
        }
    }
    return NULL;
}

bool isConnected(sockpp::connector* array_connection[]) {
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
    int16_t port = 31416;
    std::string android_address = "edu_berkeley_boinc_client_socket";
    std::string unix_address = "boinc_socket";
    std::string tcp_address = "localhost";
    std::stringstream stream;
    const int sizeBuffer = 1024;
    char buf[sizeBuffer];
    int suffix_file = 0;
    while (true)
    {
        int attemped_connections = 0;
        while (!isConnected(array_connection)) {
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
            if (isConnected(array_connection)) {
                conn = getConnected(array_connection);
            }
            std::cout << "\n";
            mySleep(1);
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

        ssize_t n = sizeBuffer;
        stream.str("");
        while (n == sizeBuffer) {
            n = conn->read(buf, sizeof(buf));
            if (isDebug) {
                std::cout << "result size: "<< n << "\n";
                std::cout << "buf: " << buf << "\n";
            }
            stream << buf;
        }
        std::string str = stream.str();
        std::size_t pos = str.find("\3");
        str = str.substr(0, pos);
        std::cout << "read bytes: " << str.size() << "\n";
        std::ofstream outFile;
        std::stringstream file_name;
        file_name.str("");
        file_name << "tal" << suffix_file << ".txt";
        outFile.open(file_name.str());
        outFile << str;
        std::cout << "write to file: tal" << suffix_file << ".txt\n\n";
        suffix_file++;
        mySleep(1);
    }
}
