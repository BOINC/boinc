// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

#include <cstdint>
#include <sys/types.h>
#ifndef _WIN32
#include <filesystem>
#include <fstream>
#include "gtest/gtest.h"
#endif
#include "crypt.h"

using namespace std;

namespace test_lib {
    class test_crypt : public ::testing::Test {
        protected:
            void SetUp() override {
                std::filesystem::create_directories(test_data_dir);
            }

            void TearDown() override {
                if (std::filesystem::exists(test_data_dir)) {
                    std::filesystem::remove_all(test_data_dir);
                }
            }

            std::filesystem::path test_data_dir =
                std::filesystem::current_path() / "test_data";
        };

    TEST_F(test_crypt, test_print_hex_data_less_than_32_bytes) {
        std::filesystem::path temp_file = test_data_dir / "temp_hex_data.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> x(sample_data, sample_data + strlen(sample_data));

        bool result = print_hex_data(f, x);
        fclose(f);

        ASSERT_TRUE(result) << "print_hex_data failed";

        std::ifstream infile(temp_file);
        std::string output((std::istreambuf_iterator<char>(infile)),
            std::istreambuf_iterator<char>());
        infile.close();

        std::string expected_output = "48656c6c6f2c20576f726c6421\n.\n";
        ASSERT_EQ(output, expected_output) <<
            "Hex output does not match expected value";
    }

    TEST_F(test_crypt, test_print_hex_data_more_than_32_bytes) {
        std::filesystem::path temp_file =
        test_data_dir / "temp_hex_data_large.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* sample_data =
            "This is a longer string that exceeds thirty-two bytes.";
        std::vector<uint8_t> x(sample_data, sample_data + strlen(sample_data));

        bool result = print_hex_data(f, x);
        fclose(f);

        ASSERT_TRUE(result) << "print_hex_data failed";

        std::ifstream infile(temp_file);
        std::string output((std::istreambuf_iterator<char>(infile)),
            std::istreambuf_iterator<char>());
        infile.close();

        std::string expected_output =
            "546869732069732061206c6f6e67657220737472696e67207468617420657863\n"
            "65656473207468697274792d74776f2062797465732e\n"
            ".\n";
        ASSERT_EQ(output, expected_output) <<
            "Hex output does not match expected value";
    }

    TEST_F(test_crypt, test_print_hex_data_empty) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_hex_data_empty.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        std::vector<uint8_t> x; // Empty vector

        bool result = print_hex_data(f, x);
        fclose(f);

        ASSERT_TRUE(result) << "print_hex_data failed";

        std::ifstream infile(temp_file);
        std::string output((std::istreambuf_iterator<char>(infile)),
            std::istreambuf_iterator<char>());
        infile.close();

        std::string expected_output = ".\n";
        ASSERT_EQ(output, expected_output) <<
            "Hex output does not match expected value";
    }

    TEST_F(test_crypt, test_print_hex_data_exactly_32_bytes) {
        std::filesystem::path temp_file = test_data_dir / "temp_hex_data_32.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* sample_data = "12345678901234567890123456789012"; // 32 bytes
        std::vector<uint8_t> x(sample_data, sample_data + strlen(sample_data));

        bool result = print_hex_data(f, x);
        fclose(f);

        ASSERT_TRUE(result) << "print_hex_data failed";

        std::ifstream infile(temp_file);
        std::string output((std::istreambuf_iterator<char>(infile)),
            std::istreambuf_iterator<char>());
        infile.close();

        std::string expected_output =
            "3132333435363738393031323334353637383930313233343536373839303132\n"
            ".\n";
        ASSERT_EQ(output, expected_output) <<
            "Hex output does not match expected value";
    }

    TEST_F(test_crypt, test_print_hex_data_return_false_if_file_is_not_opened) {
        FILE* f = nullptr; // Simulate a file that is not opened

        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> x(sample_data, sample_data + strlen(sample_data));

        bool result = print_hex_data(f, x);

        ASSERT_FALSE(result) <<
            "print_hex_data should return false if file is not opened";
    }

    TEST_F(test_crypt, test_sprint_hex_data_less_than_32_bytes) {
        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> x(sample_data, sample_data + strlen(sample_data));

        std::string result = sprint_hex_data(x);

        std::string expected_output = "48656c6c6f2c20576f726c6421\n.\n";
        ASSERT_EQ(result, expected_output) <<
            "Hex output does not match expected value";
    }

    TEST_F(test_crypt, test_sprint_hex_data_more_than_32_bytes) {
        const char* sample_data =
            "This is a longer string that exceeds thirty-two bytes.";
        std::vector<uint8_t> x(sample_data, sample_data + strlen(sample_data));

        std::string result = sprint_hex_data(x);

        std::string expected_output =
            "546869732069732061206c6f6e67657220737472696e67207468617420657863\n"
            "65656473207468697274792d74776f2062797465732e\n"
            ".\n";
        ASSERT_EQ(result, expected_output) <<
            "Hex output does not match expected value";
    }

    TEST_F(test_crypt, test_sprint_hex_data_empty) {
        std::vector<uint8_t> x; // Empty vector

        std::string result = sprint_hex_data(x);

        std::string expected_output = ".\n";
        ASSERT_EQ(result, expected_output) <<
            "Hex output does not match expected value";
    }

    TEST_F(test_crypt, test_sprint_hex_data_exactly_32_bytes) {
        // 32 bytes
        const char* sample_data = "12345678901234567890123456789012";
        std::vector<uint8_t> x(sample_data, sample_data + strlen(sample_data));

        std::string result = sprint_hex_data(x);

        std::string expected_output =
            "3132333435363738393031323334353637383930313233343536373839303132\n"
            ".\n";
        ASSERT_EQ(result, expected_output) <<
            "Hex output does not match expected value";
    }

    TEST_F(test_crypt, test_print_raw_data) {
        std::filesystem::path temp_file = test_data_dir / "temp_raw_data.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> x(sample_data, sample_data + strlen(sample_data));

        bool result = print_raw_data(f, x);
        fclose(f);

        ASSERT_TRUE(result) << "print_raw_data failed";

        std::ifstream infile(temp_file);
        std::string output((std::istreambuf_iterator<char>(infile)),
            std::istreambuf_iterator<char>());
        infile.close();

        std::string expected_output = "Hello, World!";
        ASSERT_EQ(output, expected_output) <<
            "Raw output does not match expected value";
    }

    TEST_F(test_crypt, test_print_raw_data_empty) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_raw_data_empty.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        std::vector<uint8_t> x; // Empty vector

        bool result = print_raw_data(f, x);
        fclose(f);

        ASSERT_TRUE(result) << "print_raw_data failed";

        std::ifstream infile(temp_file);
        std::string output((std::istreambuf_iterator<char>(infile)),
            std::istreambuf_iterator<char>());
        infile.close();

        std::string expected_output = ""; // Expecting empty output
        ASSERT_EQ(output, expected_output) <<
            "Raw output does not match expected value";
    }

    TEST_F(test_crypt,
        test_print_raw_data_returns_false_if_file_is_not_opened) {
        FILE* f = nullptr; // Simulate a file that is not opened

        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> x(sample_data, sample_data + strlen(sample_data));

        bool result = print_raw_data(f, x);

        ASSERT_FALSE(result) <<
            "print_raw_data should return non-zero if file is not opened";
    }

    TEST_F(test_crypt, test_scan_raw_data) {
        std::filesystem::path temp_file = test_data_dir / "temp_scan_raw_data.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* sample_data = "Hello, World!";
        fwrite(sample_data, sizeof(char), strlen(sample_data), f);
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> result = scan_raw_data(f);
        fclose(f);

        ASSERT_EQ(result.size(), strlen(sample_data)) << "Length mismatch";
        std::string result_str(result.begin(), result.end());
        ASSERT_EQ(result_str, sample_data) <<
            "Data mismatch";
    }

    TEST_F(test_crypt, test_scan_raw_data_empty) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_raw_data_empty.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> result = scan_raw_data(f);
        fclose(f);

        ASSERT_EQ(result.size(), 0) << "Length should be zero for empty file";
    }

    TEST_F(test_crypt, test_scan_raw_data_file_not_found) {
        std::filesystem::path temp_file =
            test_data_dir / "non_existent_file.txt";

        FILE* f = fopen(temp_file.string().c_str(), "r");
        ASSERT_EQ(f, nullptr) << "File should not exist";

        std::vector<uint8_t> result = scan_raw_data(f);

        ASSERT_EQ(result.size(), 0) << "Length should be zero for non-existent file";
    }

    TEST_F(test_crypt, test_scan_hex_data) {
        std::filesystem::path temp_file = test_data_dir / "temp_scan_hex_data.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* sample_hex_data = "48656c6c6f2c20576f726c6421\n.\n"; // "Hello, World!"
        fputs(sample_hex_data, f);
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> result = scan_hex_data(f);
        fclose(f);

        std::string result_str(result.begin(), result.end());
        ASSERT_EQ(result_str, "Hello, World!") << "Data mismatch";
    }

    TEST_F(test_crypt, test_scan_hex_data_empty) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_hex_data_empty.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> result = scan_hex_data(f);
        fclose(f);

        ASSERT_EQ(result.size(), 0) << "Length should be zero for empty input";
    }

    TEST_F(test_crypt, test_scan_hex_data_file_not_found) {
        std::filesystem::path temp_file =
            test_data_dir / "non_existent_hex_file.txt";

        FILE* f = fopen(temp_file.string().c_str(), "r");
        ASSERT_EQ(f, nullptr) << "File should not exist";

        std::vector<uint8_t> result = scan_hex_data(f);

        ASSERT_EQ(result.size(), 0) << "Length should be zero for non-existent file";
    }

    TEST_F(test_crypt, test_scan_hex_data_invalid_format) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_hex_data_invalid.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* invalid_hex_data = "ZZZZZZ\n.\n"; // Invalid hex characters
        fputs(invalid_hex_data, f);
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> result = scan_hex_data(f);
        fclose(f);

        ASSERT_EQ(result.size(), 0) << "Length should be zero for invalid hex input";
    }

    TEST_F(test_crypt, test_scan_hex_data_upper_case) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_hex_data_uppercase.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* uppercase_hex_data = "48656C6C6F2C20576F726C6421\n.\n"; // "Hello, World!" in uppercase
        fputs(uppercase_hex_data, f);
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> result = scan_hex_data(f);
        fclose(f);

        std::string result_str(result.begin(), result.end());
        ASSERT_EQ(result_str, "Hello, World!") << "Data mismatch on uppercase hex input";
    }

    TEST_F(test_crypt, test_scan_hex_data_mixed_case) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_hex_data_mixedcase.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* mixed_case_hex_data = "48656c6C6F2C20576F726C6421\n.\n"; // "Hello, World!" in mixed case
        fputs(mixed_case_hex_data, f);
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> result = scan_hex_data(f);
        fclose(f);

        std::string result_str(result.begin(), result.end());
        ASSERT_EQ(result_str, "Hello, World!") << "Data mismatch on mixed case hex input";
    }

    TEST_F(test_crypt, test_scan_hex_data_multiline) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_hex_data_multiline.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* multiline_hex_data =
            "48656c6c6f2c20576f726c6421\n"
            "54686973206973206120746573742e\n"
            ".\n"; // "Hello, World!This is a test."
        fputs(multiline_hex_data, f);
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> result = scan_hex_data(f);
        fclose(f);

        std::string result_str(result.begin(), result.end());
        ASSERT_EQ(result_str, "Hello, World!This is a test.") <<
            "Data mismatch on multiline hex input";
    }
}
