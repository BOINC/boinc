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
#include <chrono>
#include <thread>
#include <sockpp/socket.h>
#include <sockpp/tcp_connector.h>
#ifndef WIN32
    #include <sockpp/unix_connector.h>
#endif

void my_sleep(std::chrono::nanoseconds nanoseconds) {
    std::this_thread::sleep_for(nanoseconds);
}

sockpp::connector* get_connected(sockpp::connector* array_connection[], unsigned int arrSize) {
    if (NULL == array_connection || 0 == arrSize) {
        return NULL;
    }
    for (unsigned int i = 0; i < arrSize; ++i) {
        sockpp::connector* conn = array_connection[i];
        if (conn != NULL && conn->is_connected()) {
            return conn;
        }
    }
    return NULL;
}

bool is_connected(sockpp::connector* array_connection[], unsigned int arrSize) {
    if (NULL == array_connection || 0 == arrSize) {
        return NULL;
    }
    for (unsigned int i = 0; i < arrSize; ++i) {
        sockpp::connector* conn = array_connection[i];
        if (conn != NULL && conn->is_connected()) {
            return true;
        }
    }
    return false;
}

int main(int, char**) {
    //boinc_init();

    sockpp::socket_initializer sockInit;
    sockpp::tcp_connector tcp_conn;
    sockpp::connector* array_connection[] = { &tcp_conn, NULL };
#ifndef WIN32
    sockpp::unix_connector unix_conn;
    array_connection[1] = &unix_conn;
#endif
    sockpp::connector* conn = NULL;
    const unsigned int arrSize = sizeof(array_connection) / sizeof(array_connection[0]);
    const bool isDebug = false;
    const int port = 31416;
    const int buffer_size = 4096;
    const int wake_minutes = 30;
    const int exec_hourly = 5;
    int suffix_file_idx = 0;
    const std::string android_address = "edu_berkeley_boinc_client_socket";
    const std::string unix_address = "boinc_socket";
    const std::string tcp_address = "localhost";
    const std::string prefix_file_name = "boinc";
    const std::chrono::minutes wake_every_minutes = std::chrono::minutes(wake_minutes);
    const std::chrono::minutes exec_time_hours = std::chrono::minutes(exec_hourly);
    
    std::stringstream stream;
    char buffer[buffer_size];
    const std::chrono::time_point<std::chrono::system_clock> starttime = std::chrono::system_clock::now();
    while (true) {
        int attemped_connections = 0;
        while (!is_connected(array_connection, arrSize)) {
            ++attemped_connections;
            std::cout << "attemped connections: " << attemped_connections << "\n";
            if (!tcp_conn.connect(sockpp::inet_address(tcp_address, port))) {
                std::cout << "tcp connection fail:\n";
                std::cout << tcp_conn.last_error_str() << "\n";
                tcp_conn.clear();
            }
#if! _WIN32
            if (!is_connected(array_connection, arrSize) && !unix_conn.connect(sockpp::unix_address(unix_address))) {
                std::cout << "unix connection fail:\n";
                std::cout << unix_conn.last_error_str() << "\n";
                unix_conn.clear();                
            }

            if (!is_connected(array_connection, arrSize) && !unix_conn.connect(sockpp::unix_address(android_address))) {
                std::cout << "android unix connection fail:\n";
                std::cout << unix_conn.last_error_str() << "\n";
                unix_conn.clear();                
            }
#endif
            if (is_connected(array_connection, arrSize)) {
                conn = get_connected(array_connection, arrSize);
            }
            std::cout << "\n";
            my_sleep(std::chrono::seconds(1));
        }

        const std::string request = "<boinc_gui_rpc_request><get_state/></boinc_gui_rpc_request>\3";
        if (isDebug) {
            std::cout << "before NULL == conn: " << conn << "\n";
        }
        if(NULL == conn) {
#ifndef WIN32
            unix_conn.close();
#endif
            tcp_conn.close();
            continue;
        }
        if (isDebug) {
            std::cout << "before write: \n";
        }
        const ssize_t res = conn->write(request);

        if (isDebug) {
            std::cout << "after write: " << res << "\n";            
        }
        if (res == -1) {
            conn->close();
#ifndef WIN32
            unix_conn.close();
#endif
            tcp_conn.close();
            continue;
        }
        const std::chrono::time_point<std::chrono::system_clock> before_sleep = std::chrono::system_clock::now();
        const time_t in_time_t = std::chrono::system_clock::to_time_t(before_sleep);
        std::string time_str(20, '\0');
        std::strftime(&time_str[0], time_str.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&in_time_t));
        std::cout << "now: " << time_str << "\n";
        std::cout << "request bytes: " << request.size() << "\n";
        std::cout << "write bytes: " << res << "\n";

        if (isDebug) {
            std::cout << "after write, before read: \n";
        }

        size_t n = buffer_size;
        stream.str("");
        while (n == buffer_size) {
            n = conn->read(buffer, sizeof(buffer));
            if (isDebug) {
                std::cout << "result size: " << n << "\n";
                std::cout << "buffer: " << buffer << "\n";
            }
            if (n > 0) {
                std::string str(buffer, buffer + n);
                stream << str;
            }
        }
        std::string str = stream.str();
        str = str.substr(0, str.size() - 1);
        std::cout << "read bytes: " << str.size() << "\n";
        if(str.empty()) {
            continue;
        }
        std::ofstream outFile;
        std::stringstream file_name;
        file_name.str("");
        file_name << prefix_file_name << suffix_file_idx << ".txt";
        outFile.open(file_name.str());
        outFile << str;
        std::cout << "write to file: " << prefix_file_name << suffix_file_idx << ".txt\n\n";
        ++suffix_file_idx;
        outFile.flush();
        outFile.close();
        const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        const std::chrono::system_clock::duration time_to_sleep = wake_every_minutes - ((now - starttime) % wake_every_minutes);
        if (now - starttime >= exec_time_hours) {
            break;
        }
        my_sleep(time_to_sleep);
    }
}
