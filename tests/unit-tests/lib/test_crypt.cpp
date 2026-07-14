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
#include <memory>
#include <sys/types.h>
#ifndef _WIN32
#include <filesystem>
#include <fstream>
#include "gtest/gtest.h"
#endif
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/core_names.h>

#include "crypt.h"

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

            EVP_PKEY* generate_rsa_key() {
                EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
                if (!ctx) {
                    return nullptr;
                }
                std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>
                    ctx_guard(ctx, EVP_PKEY_CTX_free);
                if (EVP_PKEY_keygen_init(ctx) <= 0) {
                    return nullptr;
                }
                if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 1024) <= 0) {
                    return nullptr;
                }

                EVP_PKEY* pkey = nullptr;
                if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
                    return nullptr;
                }

                return pkey;
            }

            bool fill_keys_from_evp(EVP_PKEY* pkey,
                R_RSA_PRIVATE_KEY& private_key, R_RSA_PUBLIC_KEY& public_key) {
                auto bn_to_bin = [](const BIGNUM* bn, unsigned char* buffer,
                    size_t buffer_size) {
                    int num_bytes = BN_num_bytes(bn);
                    if (num_bytes > static_cast<int>(buffer_size)) {
                        throw std::runtime_error("Buffer too small for BIGNUM");
                    }
                    memset(buffer, 0, buffer_size);
                    BN_bn2bin(bn, buffer + (buffer_size - num_bytes));
                };

                BIGNUM *n = nullptr, *e = nullptr, *d = nullptr, *p = nullptr,
                    *q = nullptr, *dmp1 = nullptr, *dmq1 = nullptr,
                    *iqmp = nullptr;

                if (!EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_N, &n) ||
                    !EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_E, &e)) {
                    return false;
                }
                EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_D,
                    &d);
                EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_FACTOR1,
                    &p);
                EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_FACTOR2,
                    &q);
                EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_EXPONENT1,
                    &dmp1);
                EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_EXPONENT2,
                    &dmq1);
                EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_COEFFICIENT,
                    &iqmp);

                public_key.bits = 1024;
                bn_to_bin(n, public_key.modulus, sizeof(public_key.modulus));
                bn_to_bin(e, public_key.exponent, sizeof(public_key.exponent));

                memset(&private_key, 0, sizeof(private_key));
                private_key.bits = 1024;
                bn_to_bin(n, private_key.modulus, sizeof(private_key.modulus));
                bn_to_bin(e, private_key.publicExponent,
                    sizeof(private_key.publicExponent));
                if (d) {
                    bn_to_bin(d, private_key.exponent,
                        sizeof(private_key.exponent));
                }
                if (p) {
                    bn_to_bin(p, private_key.prime[0],
                        sizeof(private_key.prime[0]));
                }
                if (q) {
                    bn_to_bin(q, private_key.prime[1],
                        sizeof(private_key.prime[1]));
                }
                if (dmp1) {
                    bn_to_bin(dmp1, private_key.primeExponent[0],
                        sizeof(private_key.primeExponent[0]));
                }
                if (dmq1) {
                    bn_to_bin(dmq1, private_key.primeExponent[1],
                        sizeof(private_key.primeExponent[1]));
                }
                if (iqmp) {
                    bn_to_bin(iqmp, private_key.coefficient,
                        sizeof(private_key.coefficient));
                }

                BN_free(n);
                BN_free(e);
                if (d) {
                    BN_free(d);
                }
                if (p) {
                    BN_free(p);
                }
                if (q) {
                    BN_free(q);
                }
                if (dmp1) {
                    BN_free(dmp1);
                }
                if (dmq1) {
                    BN_free(dmq1);
                }
                if (iqmp) {
                    BN_free(iqmp);
                }

                return true;
            }

            std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data,
                EVP_PKEY* private_key) {
                // encrypt data with RSA_PKCS1_PADDING
                std::vector<uint8_t> encrypted_data(EVP_PKEY_size(private_key));
                EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(private_key, nullptr);
                if (!ctx) {
                    throw std::runtime_error("Failed to create EVP_PKEY_CTX");
                }
                std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>
                    ctx_guard(ctx, EVP_PKEY_CTX_free);
                if (EVP_PKEY_sign_init(ctx) <= 0) {
                    throw std::runtime_error("Failed to initialize encryption");
                }
                if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
                    throw std::runtime_error("Failed to set RSA padding");
                }
                size_t outlen = encrypted_data.size();
                if (EVP_PKEY_sign(ctx, encrypted_data.data(), &outlen,
                    data.data(), data.size()) <= 0) {
                    return {};
                }
                encrypted_data.resize(outlen);
                return encrypted_data;
            }

            std::vector<uint8_t> decrypt(
                const std::vector<uint8_t>& encrypted_data,
                EVP_PKEY* public_key) {
                std::vector<uint8_t> decrypted_data(EVP_PKEY_size(public_key));
                EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(public_key, nullptr);
                if (!ctx) {
                    throw std::runtime_error("Failed to create EVP_PKEY_CTX");
                }
                std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>
                    ctx_guard(ctx, EVP_PKEY_CTX_free);
                if (EVP_PKEY_verify_recover_init(ctx) <= 0) {
                    throw std::runtime_error("Failed to initialize decryption");
                }
                if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
                    throw std::runtime_error("Failed to set RSA padding");
                }
                size_t outlen = decrypted_data.size();
                if (EVP_PKEY_verify_recover(ctx, decrypted_data.data(), &outlen,
                    encrypted_data.data(), encrypted_data.size()) <= 0) {
                    return {};
                }
                decrypted_data.resize(outlen);
                return decrypted_data;
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
        std::filesystem::path temp_file =
            test_data_dir / "temp_hex_data_32.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        // 32 bytes
        const char* sample_data = "12345678901234567890123456789012";
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
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_raw_data.txt";
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

        ASSERT_EQ(result.size(), 0)
            << "Length should be zero for non-existent file";
    }

    TEST_F(test_crypt, test_scan_hex_data) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_hex_data.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        // "Hello, World!"
        const char* sample_hex_data = "48656c6c6f2c20576f726c6421\n.\n";
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

        ASSERT_EQ(result.size(), 0)
            << "Length should be zero for non-existent file";
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

        ASSERT_EQ(result.size(), 0)
            << "Length should be zero for invalid hex input";
    }

    TEST_F(test_crypt, test_scan_hex_data_upper_case) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_hex_data_uppercase.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        // "Hello, World!" in uppercase
        const char* uppercase_hex_data = "48656C6C6F2C20576F726C6421\n.\n";
        fputs(uppercase_hex_data, f);
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> result = scan_hex_data(f);
        fclose(f);

        std::string result_str(result.begin(), result.end());
        ASSERT_EQ(result_str, "Hello, World!")
            << "Data mismatch on uppercase hex input";
    }

    TEST_F(test_crypt, test_scan_hex_data_mixed_case) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_hex_data_mixedcase.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        // "Hello, World!" in mixed case
        const char* mixed_case_hex_data = "48656c6C6F2C20576F726C6421\n.\n";
        fputs(mixed_case_hex_data, f);
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> result = scan_hex_data(f);
        fclose(f);

        std::string result_str(result.begin(), result.end());
        ASSERT_EQ(result_str, "Hello, World!")
            << "Data mismatch on mixed case hex input";
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

    TEST_F(test_crypt, test_print_key_hex) {
        std::filesystem::path temp_file = test_data_dir / "temp_key_hex.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const short int key_data_size = 32; // Example size for key data
        unsigned char key_data[key_data_size + sizeof(key_data_size)];
        KEY *key = reinterpret_cast<KEY*>(key_data);
        key->bits = 256; // Example bit size
        for (size_t i = 0; i < key_data_size; ++i) {
            key->data[i] = static_cast<unsigned char>(i);
        }

        bool result = print_key_hex(f, key, sizeof(key_data));
        fclose(f);

        ASSERT_TRUE(result) << "print_key_hex failed";

        std::ifstream infile(temp_file);
        std::string output((std::istreambuf_iterator<char>(infile)),
            std::istreambuf_iterator<char>());
        infile.close();

        std::string expected_output =
            "256\n"
            "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"
            "\n.\n";
        ASSERT_EQ(output, expected_output) <<
            "Key hex output does not match expected value";
    }

    TEST_F(test_crypt, test_print_key_hex_file_not_opened) {
        FILE* f = nullptr; // Simulate a file that is not opened

        const short int key_data_size = 32; // Example size for key data
        unsigned char key_data[key_data_size + sizeof(key_data_size)];
        KEY *key = reinterpret_cast<KEY*>(key_data);
        key->bits = 256; // Example bit size
        for (size_t i = 0; i < key_data_size; ++i) {
            key->data[i] = static_cast<unsigned char>(i);
        }

        bool result = print_key_hex(f, key, sizeof(key_data));

        ASSERT_FALSE(result) <<
            "print_key_hex should return non-zero if file is not opened";
    }

    TEST_F(test_crypt, test_print_key_hex_invalid_key) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_key_hex_invalid.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        KEY *key = nullptr; // Simulate an invalid key

        bool result = print_key_hex(f, key, sizeof(KEY));
        fclose(f);

        ASSERT_FALSE(result) <<
            "print_key_hex should return non-zero for invalid key";
    }

    TEST_F(test_crypt, test_print_key_hex_zero_size) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_key_hex_zero_size.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const short int key_data_size = 32; // Example size for key data
        unsigned char key_data[key_data_size + sizeof(key_data_size)];
        KEY *key = reinterpret_cast<KEY*>(key_data);
        key->bits = 256; // Example bit size
        for (size_t i = 0; i < key_data_size; ++i) {
            key->data[i] = static_cast<unsigned char>(i);
        }

        bool result = print_key_hex(f, key, 0); // Zero size
        fclose(f);

        ASSERT_FALSE(result) <<
            "print_key_hex should return non-zero for zero size";
    }

    TEST_F(test_crypt, test_scan_key_hex) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_key_hex.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const short int key_data_size = 32; // Example size for key data
        unsigned char key_data[key_data_size + sizeof(key_data_size)];
        KEY *key = reinterpret_cast<KEY*>(key_data);
        key->bits = 256; // Example bit size
        for (size_t i = 0; i < key_data_size; ++i) {
            key->data[i] = static_cast<unsigned char>(i);
        }

        bool print_result = print_key_hex(f, key, sizeof(key_data));
        fclose(f);
        ASSERT_TRUE(print_result) << "print_key_hex failed";

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> scan_result = scan_key_hex(f);
        fclose(f);
        KEY *scanned_key = reinterpret_cast<KEY*>(scan_result.data());

        ASSERT_EQ(scanned_key->bits, key->bits) << "Key bits mismatch";
        ASSERT_EQ(memcmp(scanned_key->data, key->data, key_data_size), 0) <<
            "Key data mismatch";
    }

    TEST_F(test_crypt, test_scan_key_hex_file_not_opened) {
        FILE* f = nullptr; // Simulate a file that is not opened

        std::vector<uint8_t> result = scan_key_hex(f);

        ASSERT_TRUE(result.empty()) <<
            "scan_key_hex should return empty vector if file is not opened";
    }

    TEST_F(test_crypt, test_scan_key_hex_invalid_key) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_key_hex_invalid.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* invalid_key_data = "InvalidKeyData\n.\n";
        fputs(invalid_key_data, f);
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> scan_result = scan_key_hex(f);
        fclose(f);

        ASSERT_TRUE(scan_result.empty()) <<
            "scan_key_hex should return empty vector for invalid key data";
    }

    TEST_F(test_crypt, test_scan_key_hex_file_not_found) {
        std::filesystem::path temp_file =
            test_data_dir / "non_existent_key_file.txt";

        FILE* f = fopen(temp_file.string().c_str(), "r");
        ASSERT_EQ(f, nullptr) << "File should not exist";

        std::vector<uint8_t> scan_result = scan_key_hex(f);

        ASSERT_TRUE(scan_result.empty()) <<
            "scan_key_hex should return empty vector for non-existent file";
    }

    TEST_F(test_crypt, test_scan_key_hex_invalid_format) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_key_hex_invalid_format.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        // Invalid hex characters
        const char* invalid_key_data = "256\nZZZZZZ\n.\n";
        fputs(invalid_key_data, f);
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> scan_result = scan_key_hex(f);
        fclose(f);

        ASSERT_TRUE(scan_result.empty()) <<
            "scan_key_hex should return empty vector for invalid hex input";
    }

    TEST_F(test_crypt, test_scan_key_hex_multiline) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_key_hex_multiline.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* multiline_key_data = "256\n"
            "0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f\n"
            "202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f40"
            "\n.\n"; // Example multiline key data
        fputs(multiline_key_data, f);
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> scan_result = scan_key_hex(f);
        fclose(f);

        KEY *scanned_key = reinterpret_cast<KEY*>(scan_result.data());
        ASSERT_EQ(scanned_key->bits, 256) << "Key bits mismatch";
        for (size_t i = 0; i < 32; ++i) {
            ASSERT_EQ(scanned_key->data[i], static_cast<unsigned char>(i + 1))
                << "Key data mismatch at index " << i;
        }
    }

    TEST_F(test_crypt, test_scan_key_hex_upper_case) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_key_hex_uppercase.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* uppercase_key_data = "256\n"
            "0102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F20\n"
            ".\n"; // Example uppercase key data
        fputs(uppercase_key_data, f);
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> scan_result = scan_key_hex(f);
        fclose(f);

        KEY *scanned_key = reinterpret_cast<KEY*>(scan_result.data());
        ASSERT_EQ(scanned_key->bits, 256) << "Key bits mismatch";
        for (size_t i = 0; i < 32; ++i) {
            ASSERT_EQ(scanned_key->data[i], static_cast<unsigned char>(i + 1))
                << "Key data mismatch at index " << i;
        }
    }

    TEST_F(test_crypt, test_scan_key_hex_mixed_case) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_key_hex_mixedcase.txt";
        FILE* f = fopen(temp_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* mixed_case_key_data = "256\n"
            "0102030405060708090a0B0C0D0E0F101112131415161718191A1B1C1D1E1F20\n"
            ".\n"; // Example mixed case key data
        fputs(mixed_case_key_data, f);
        fclose(f);

        f = fopen(temp_file.string().c_str(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        std::vector<uint8_t> scan_result = scan_key_hex(f);
        fclose(f);

        KEY *scanned_key = reinterpret_cast<KEY*>(scan_result.data());
        ASSERT_EQ(scanned_key->bits, 256) << "Key bits mismatch";
        for (size_t i = 0; i < 32; ++i) {
            ASSERT_EQ(scanned_key->data[i], static_cast<unsigned char>(i + 1))
                << "Key data mismatch at index " << i;
        }
    }

    TEST_F(test_crypt, test_encrypt_private_and_decrypt_public) {
        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> data(sample_data,
            sample_data + strlen(sample_data));

        std::vector<uint8_t> encrypted_data = encrypt(data, private_key);
        ASSERT_FALSE(encrypted_data.empty()) << "Encryption failed";

        DATA_BLOCK input_block;
        input_block.len = data.size();
        input_block.data = data.data();

        const size_t output_block_size = encrypted_data.size();
        std::vector<uint8_t> encrypted_data_for_testing(output_block_size, 0);
        DATA_BLOCK output_block;
        output_block.len = encrypted_data_for_testing.size();
        output_block.data = encrypted_data_for_testing.data();

        int encrypt_result = encrypt_private(private_key_struct, input_block,
            output_block);
        ASSERT_EQ(encrypt_result, 0) << "encrypt_private failed";

        // compare encrypted_data with encrypted_data_for_testing
        ASSERT_EQ(encrypted_data.size(), encrypted_data_for_testing.size())
            << "Encrypted data length mismatch";
        ASSERT_EQ(encrypted_data, encrypted_data_for_testing)
            << "Encrypted data mismatch";

        // Now decrypt the data encrypted by this test
        // with the method in this test
        std::vector<uint8_t> decrypted_data =
            decrypt(encrypted_data, private_key);
        ASSERT_EQ(decrypted_data, data)
            << "Decrypted data does not match original";

        // Now decrypt the data encrypted by the method under test
        // with the method under test
        DATA_BLOCK decrypted_block;
        decrypted_block.len = data.size();
        std::vector<uint8_t> decrypted_data_for_testing(decrypted_block.len, 0);
        decrypted_block.data = decrypted_data_for_testing.data();

        int decrypt_result = decrypt_public(public_key_struct, output_block,
            decrypted_block);
        ASSERT_EQ(decrypt_result, 0) << "decrypt_public failed";
        ASSERT_EQ(decrypted_data_for_testing, data)
            << "Decrypted data from method under test does not match original";
    }

    TEST_F(test_crypt, test_encrypt_decrypt_with_different_keys) {
        EVP_PKEY* private_key1 = generate_rsa_key();
        ASSERT_NE(private_key1, nullptr) << "Failed to generate RSA key 1";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key1_guard(private_key1, EVP_PKEY_free);

        EVP_PKEY* private_key2 = generate_rsa_key();
        ASSERT_NE(private_key2, nullptr) << "Failed to generate RSA key 2";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key2_guard(private_key2, EVP_PKEY_free);

        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> data(sample_data,
            sample_data + strlen(sample_data));

        std::vector<uint8_t> encrypted_data = encrypt(data, private_key1);
        ASSERT_FALSE(encrypted_data.empty()) << "Encryption failed";

        // Attempt to decrypt with a different public key
        std::vector<uint8_t> decrypted_data =
            decrypt(encrypted_data, private_key2);

        // The decryption should fail or produce incorrect data
        ASSERT_NE(decrypted_data, data)
            << "Decryption with a different key should not match original data";
    }

    TEST_F(test_crypt, test_private_to_openssl) {
        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        auto converted_private_key =
            private_to_openssl(private_key_struct);
        ASSERT_NE(converted_private_key.get(), nullptr)
            << "private_to_openssl failed";

        // test that the converted private key can be used to encrypt and decrypt data
        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> data(sample_data,
            sample_data + strlen(sample_data));

        std::vector<uint8_t> encrypted_data = encrypt(data, converted_private_key.get());
        ASSERT_FALSE(encrypted_data.empty()) << "Encryption failed";
        std::vector<uint8_t> decrypted_data = decrypt(encrypted_data, private_key);
        ASSERT_EQ(decrypted_data, data) << "Decrypted data does not match original";
    }

    TEST_F(test_crypt, test_public_to_openssl) {
        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        auto converted_public_key =
            public_to_openssl(public_key_struct);
        ASSERT_NE(converted_public_key.get(), nullptr)
            << "public_to_openssl failed";

        // test that the converted public key can be used to encrypt and decrypt data
        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> data(sample_data,
            sample_data + strlen(sample_data));

        std::vector<uint8_t> encrypted_data = encrypt(data, private_key);
        ASSERT_FALSE(encrypted_data.empty()) << "Encryption failed";
        std::vector<uint8_t> decrypted_data = decrypt(encrypted_data, converted_public_key.get());
        ASSERT_EQ(decrypted_data, data) << "Decrypted data does not match original";
    }
}
