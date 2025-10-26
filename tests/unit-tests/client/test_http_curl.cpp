// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "http_curl.h"

using namespace std;

namespace test_http_curl {
    class test_http_curl : public ::testing::Test {};

    TEST_F(test_http_curl, test_user_agent_string) {
        ASSERT_NE(string(""), HTTP_CURL::instance().get_user_agent_string());
        ASSERT_EQ(HTTP_CURL::instance().get_user_agent_string(), HTTP_CURL::instance().get_user_agent_string());
    }

    TEST_F(test_http_curl, test_curl_multi_handle) {
        ASSERT_NE(nullptr, HTTP_CURL::instance().get_curl_multi_handle());
        ASSERT_EQ(HTTP_CURL::instance().get_curl_multi_handle(), HTTP_CURL::instance().get_curl_multi_handle());
    }

    TEST_F(test_http_curl, test_http_1_0_flag) {
        // default is false
        ASSERT_EQ(false, HTTP_CURL::instance().get_use_http_1_0());
        HTTP_CURL::instance().set_use_http_1_0();
        ASSERT_EQ(true, HTTP_CURL::instance().get_use_http_1_0());
    }

    TEST_F(test_http_curl, test_atomic_trace_count) {
        constexpr auto num_threads = 10u;
        constexpr auto calls_per_thread = 100'000u;
        vector<thread> threads;
        threads.reserve(num_threads);
        const auto start_trace_id = HTTP_CURL::instance().get_next_trace_id();
        for (auto i = 0u; i < num_threads; ++i) {
            threads.emplace_back([]() {
                for (auto j = 0u; j < calls_per_thread; ++j) {
                    HTTP_CURL::instance().get_next_trace_id();
                }
            });
        }
        for (auto& t : threads) {
            t.join();
        }
        constexpr auto expected = num_threads * calls_per_thread;
        const auto actual = HTTP_CURL::instance().get_next_trace_id();
        EXPECT_EQ(expected + start_trace_id + 1, actual);
    }
}
