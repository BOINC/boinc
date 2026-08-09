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
#include <functional>
#include <vector>
#ifndef _WIN32
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include "gtest/gtest.h"
#endif
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/rsa.h>
#include <openssl/core_names.h>

#include "str_replace.h"
#include "cert_sig.h"
#include "crypt.h"

namespace test_lib {
    class test_crypt : public ::testing::Test {
        protected:
            void SetUp() override {
                std::filesystem::create_directories(test_data_dir);
            }

            struct file_closer {
                void operator()(FILE* f) const { if (f) fclose(f); }
            };
            using unique_FILE = std::unique_ptr<FILE, file_closer>;

            void TearDown() override {
                if (std::filesystem::exists(test_data_dir)) {
                    size_t retry_count = 0;
                    while (retry_count < 5) {
                        try {
                            std::filesystem::remove_all(test_data_dir);
                            break;
                        }
                        catch (const std::filesystem::filesystem_error& e) {
                            std::cerr << "Attempt " << (retry_count + 1) <<
                                " to remove test data directory failed: " <<
                                e.what() << std::endl;
                            ++retry_count;
                            std::this_thread::sleep_for(
                                std::chrono::seconds(1));
                        }
                    }
                    if (retry_count == 5) {
                        std::cerr <<
                            "Failed to remove test data directory "
                            "after 5 attempts." << std::endl;
                        throw std::runtime_error(
                            "Failed to remove test data directory "
                            "after 5 attempts.");
                    }
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

            FILE* open_file(const std::string& filename,
                const std::string& filemode) {
#ifdef _WIN32
                FILE* f = nullptr;
                if (fopen_s(&f, filename.c_str(), filemode.c_str()) != 0) {
                    return nullptr;
                }
                return f;
#else
                return fopen(filename.c_str(), filemode.c_str());
#endif
            }

            FILE* open_tmpfile() {
#ifdef _WIN32
                FILE* f = nullptr;
                if (tmpfile_s(&f) != 0) {
                    return nullptr;
                }
                return f;
#else
                return tmpfile();
#endif
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
                EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_COEFFICIENT1,
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

            std::string get_md5(const std::string& data) {
                EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
                if (!mdctx) {
                    throw std::runtime_error("Failed to create EVP_MD_CTX");
                }
                std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>
                    mdctx_guard(mdctx, EVP_MD_CTX_free);
                if (EVP_DigestInit_ex2(mdctx, EVP_md5(), nullptr) <= 0) {
                    throw std::runtime_error("Failed to initialize MD5 digest");
                }
                if (EVP_DigestUpdate(mdctx, data.data(), data.size()) <= 0) {
                    throw std::runtime_error("Failed to update MD5 digest");
                }
                unsigned char md_value[EVP_MAX_MD_SIZE];
                unsigned int md_len = 0;
                if (EVP_DigestFinal_ex(mdctx, md_value, &md_len) <= 0) {
                    throw std::runtime_error("Failed to finalize MD5 digest");
                }
                std::string md5_hex;
                for (unsigned int i = 0; i < md_len; ++i) {
                    char buf[3];
                    snprintf(buf, sizeof(buf), "%02x", md_value[i]);
                    md5_hex += buf;
                }
                return md5_hex;
            }

            std::filesystem::path test_data_dir =
                std::filesystem::current_path() / "test_data";
        };

    TEST_F(test_crypt, test_print_hex_data_less_than_32_bytes) {
        std::filesystem::path temp_file = test_data_dir / "temp_hex_data.txt";
        FILE* f = open_file(temp_file.string(), "w");
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
        FILE* f = open_file(temp_file.string(), "w");
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
        FILE* f = open_file(temp_file.string(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        std::vector<uint8_t> x; // Empty vector

        bool result = print_hex_data(f, x);
        fclose(f);

        ASSERT_FALSE(result) << "print_hex_data failed";
    }

    TEST_F(test_crypt, test_print_hex_data_exactly_32_bytes) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_hex_data_32.txt";
        FILE* f = open_file(temp_file.string(), "w");
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

        auto[error, result] = sprint_hex_data(x);
        ASSERT_TRUE(error) << "sprint_hex_data failed";

        std::string expected_output = "48656c6c6f2c20576f726c6421\n.\n";
        ASSERT_EQ(result, expected_output) <<
            "Hex output does not match expected value";
    }

    TEST_F(test_crypt, test_sprint_hex_data_more_than_32_bytes) {
        const char* sample_data =
            "This is a longer string that exceeds thirty-two bytes.";
        std::vector<uint8_t> x(sample_data, sample_data + strlen(sample_data));

        auto[error, result] = sprint_hex_data(x);
        ASSERT_TRUE(error) << "sprint_hex_data failed";

        std::string expected_output =
            "546869732069732061206c6f6e67657220737472696e67207468617420657863\n"
            "65656473207468697274792d74776f2062797465732e\n"
            ".\n";
        ASSERT_EQ(result, expected_output) <<
            "Hex output does not match expected value";
    }

    TEST_F(test_crypt, test_sprint_hex_data_empty) {
        std::vector<uint8_t> x; // Empty vector

        auto[error, result] = sprint_hex_data(x);
        ASSERT_FALSE(error) << "sprint_hex_data failed";
    }

    TEST_F(test_crypt, test_sprint_hex_data_exactly_32_bytes) {
        // 32 bytes
        const char* sample_data = "12345678901234567890123456789012";
        std::vector<uint8_t> x(sample_data, sample_data + strlen(sample_data));

        auto[error, result] = sprint_hex_data(x);
        ASSERT_TRUE(error) << "sprint_hex_data failed";

        std::string expected_output =
            "3132333435363738393031323334353637383930313233343536373839303132\n"
            ".\n";
        ASSERT_EQ(result, expected_output) <<
            "Hex output does not match expected value";
    }

    TEST_F(test_crypt, test_print_raw_data) {
        std::filesystem::path temp_file = test_data_dir / "temp_raw_data.txt";
        FILE* f = open_file(temp_file.string(), "w");
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
        FILE* f = open_file(temp_file.string(), "w");
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
        FILE* f = open_file(temp_file.string(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* sample_data = "Hello, World!";
        fwrite(sample_data, sizeof(char), strlen(sample_data), f);
        fclose(f);

        f = open_file(temp_file.string(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        auto [error, result] = scan_raw_data(f);
        fclose(f);
        ASSERT_TRUE(error) << "scan_raw_data failed";

        ASSERT_EQ(result.size(), strlen(sample_data)) << "Length mismatch";
        std::string result_str(result.begin(), result.end());
        ASSERT_EQ(result_str, sample_data) <<
            "Data mismatch";
    }

    TEST_F(test_crypt, test_scan_raw_data_empty) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_raw_data_empty.txt";
        FILE* f = open_file(temp_file.string(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";
        fclose(f);

        f = open_file(temp_file.string(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        auto [error, result] = scan_raw_data(f);
        fclose(f);
        ASSERT_TRUE(error) << "scan_raw_data failed";

        ASSERT_EQ(result.size(), 0) << "Length should be zero for empty file";
    }

    TEST_F(test_crypt, test_scan_raw_data_file_not_found) {
        std::filesystem::path temp_file =
            test_data_dir / "non_existent_file.txt";

        FILE* f = open_file(temp_file.string(), "r");
        ASSERT_EQ(f, nullptr) << "File should not exist";

        auto [error, result] = scan_raw_data(f);
        ASSERT_FALSE(error) << "scan_raw_data should fail for non-existent file";

        ASSERT_EQ(result.size(), 0)
            << "Length should be zero for non-existent file";
    }

    TEST_F(test_crypt, test_scan_hex_data) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_hex_data.txt";
        FILE* f = open_file(temp_file.string(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        // "Hello, World!"
        const char* sample_hex_data = "48656c6c6f2c20576f726c6421\n.\n";
        fputs(sample_hex_data, f);
        fclose(f);

        f = open_file(temp_file.string(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        auto [error, result] = scan_hex_data(f);
        fclose(f);
        ASSERT_TRUE(error) << "scan_hex_data failed";

        std::string result_str(result.begin(), result.end());
        ASSERT_EQ(result_str, "Hello, World!") << "Data mismatch";
    }

    TEST_F(test_crypt, test_scan_hex_data_empty) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_hex_data_empty.txt";
        FILE* f = open_file(temp_file.string(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";
        fclose(f);

        f = open_file(temp_file.string(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        auto [error, result] = scan_hex_data(f);
        fclose(f);
        ASSERT_FALSE(error) << "scan_hex_data failed";

        ASSERT_EQ(result.size(), 0) << "Length should be zero for empty input";
    }

    TEST_F(test_crypt, test_scan_hex_data_file_not_found) {
        std::filesystem::path temp_file =
            test_data_dir / "non_existent_hex_file.txt";

        FILE* f = open_file(temp_file.string(), "r");
        ASSERT_EQ(f, nullptr) << "File should not exist";

        auto [error, result] = scan_hex_data(f);
        ASSERT_FALSE(error) << "scan_hex_data should fail for non-existent file";

        ASSERT_EQ(result.size(), 0)
            << "Length should be zero for non-existent file";
    }

    TEST_F(test_crypt, test_scan_hex_data_invalid_format) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_hex_data_invalid.txt";
        FILE* f = open_file(temp_file.string(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* invalid_hex_data = "ZZZZZZ\n.\n"; // Invalid hex characters
        fputs(invalid_hex_data, f);
        fclose(f);

        f = open_file(temp_file.string(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        auto [error, result] = scan_hex_data(f);
        fclose(f);
        ASSERT_TRUE(error) << "scan_hex_data should fail for invalid hex input";

        ASSERT_EQ(result.size(), 0)
            << "Length should be zero for invalid hex input";
    }

    TEST_F(test_crypt, test_scan_hex_data_upper_case) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_hex_data_uppercase.txt";
        FILE* f = open_file(temp_file.string(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        // "Hello, World!" in uppercase
        const char* uppercase_hex_data = "48656C6C6F2C20576F726C6421\n.\n";
        fputs(uppercase_hex_data, f);
        fclose(f);

        f = open_file(temp_file.string(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        auto [error, result] = scan_hex_data(f);
        fclose(f);
        ASSERT_TRUE(error) << "scan_hex_data failed for uppercase hex input";

        std::string result_str(result.begin(), result.end());
        ASSERT_EQ(result_str, "Hello, World!")
            << "Data mismatch on uppercase hex input";
    }

    TEST_F(test_crypt, test_scan_hex_data_mixed_case) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_hex_data_mixedcase.txt";
        FILE* f = open_file(temp_file.string(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        // "Hello, World!" in mixed case
        const char* mixed_case_hex_data = "48656c6C6F2C20576F726C6421\n.\n";
        fputs(mixed_case_hex_data, f);
        fclose(f);

        f = open_file(temp_file.string(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        auto [error, result] = scan_hex_data(f);
        fclose(f);
        ASSERT_TRUE(error) << "scan_hex_data failed for mixed case hex input";

        std::string result_str(result.begin(), result.end());
        ASSERT_EQ(result_str, "Hello, World!")
            << "Data mismatch on mixed case hex input";
    }

    TEST_F(test_crypt, test_scan_hex_data_multiline) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_scan_hex_data_multiline.txt";
        FILE* f = open_file(temp_file.string(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* multiline_hex_data =
            "48656c6c6f2c20576f726c6421\n"
            "54686973206973206120746573742e\n"
            ".\n"; // "Hello, World!This is a test."
        fputs(multiline_hex_data, f);
        fclose(f);

        f = open_file(temp_file.string(), "r");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";

        auto [error, result] = scan_hex_data(f);
        fclose(f);
        ASSERT_TRUE(error) << "scan_hex_data failed for multiline hex input";

        std::string result_str(result.begin(), result.end());
        ASSERT_EQ(result_str, "Hello, World!This is a test.") <<
            "Data mismatch on multiline hex input";
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

        auto [result, encrypted_data_for_testing] =
            encrypt_private(private_key_struct, data);
        ASSERT_TRUE(result) << "Encryption failed";

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

        std::vector<uint8_t> decrypt_result;
        std::tie(result, decrypt_result) =
            decrypt_public(public_key_struct, encrypted_data_for_testing);
        ASSERT_TRUE(result) << "Decryption failed";
        ASSERT_FALSE(decrypt_result.empty()) << "Decryption failed";
        ASSERT_EQ(decrypt_result, data)
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

        auto [result, converted_private_key] =
            private_to_openssl(private_key_struct);
        ASSERT_TRUE(result) << "private_to_openssl failed";
        ASSERT_NE(converted_private_key.get(), nullptr)
            << "private_to_openssl failed";

        // test that the converted private key can be used
        // to encrypt and decrypt data
        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> data(sample_data,
            sample_data + strlen(sample_data));

        std::vector<uint8_t> encrypted_data =
            encrypt(data, converted_private_key.get());
        ASSERT_FALSE(encrypted_data.empty()) << "Encryption failed";
        std::vector<uint8_t> decrypted_data =
            decrypt(encrypted_data, private_key);
        ASSERT_EQ(decrypted_data, data) <<
            "Decrypted data does not match original";
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

        auto [result, converted_public_key] =
            public_to_openssl(public_key_struct);
        ASSERT_TRUE(result)
            << "public_to_openssl failed";
        ASSERT_NE(converted_public_key.get(), nullptr)
            << "public_to_openssl failed";

        // test that the converted public key can be used
        // to encrypt and decrypt data
        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> data(sample_data,
            sample_data + strlen(sample_data));

        std::vector<uint8_t> encrypted_data = encrypt(data, private_key);
        ASSERT_FALSE(encrypted_data.empty()) << "Encryption failed";
        std::vector<uint8_t> decrypted_data =
            decrypt(encrypted_data, converted_public_key.get());
        ASSERT_EQ(decrypted_data, data) <<
            "Decrypted data does not match original";
    }

    TEST_F(test_crypt, test_sign_file) {
        std::filesystem::path temp_file = test_data_dir / "temp_sign_file.txt";
        FILE* f = open_file(temp_file.string(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* sample_data = "Hello, World!";
        fwrite(sample_data, sizeof(char), strlen(sample_data), f);
        fclose(f);

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        auto [result, signature_data] =
            sign_file(temp_file.string(), private_key_struct);
        ASSERT_TRUE(result) << "Signing failed";
        ASSERT_FALSE(signature_data.empty()) << "Signing failed";

        // calculate the MD5 hash of the file and compare with signature
        std::string file_hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> file_hash_vec(file_hash.begin(), file_hash.end());
        auto encrypted = encrypt(file_hash_vec, private_key);
        ASSERT_EQ(encrypted.size(), signature_data.size()) <<
            "Signature length does not match file hash length";
        ASSERT_EQ(encrypted, signature_data) <<
            "Signature does not match file hash";
    }

    TEST_F(test_crypt, test_sign_file_invalid_file) {
        std::filesystem::path temp_file =
            test_data_dir / "non_existent_file.txt";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        auto [result, signature_data] =
            sign_file(temp_file.string(), private_key_struct);
        ASSERT_FALSE(result) <<
            "sign_file should fail for non-existent file";
        ASSERT_TRUE(signature_data.empty()) <<
            "sign_file should fail for non-existent file";
    }

    TEST_F(test_crypt, test_sign_file_invalid_key) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_sign_file_invalid_key.txt";
        FILE* f = open_file(temp_file.string(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        const char* sample_data = "Hello, World!";
        fwrite(sample_data, sizeof(char), strlen(sample_data), f);
        fclose(f);

        R_RSA_PRIVATE_KEY private_key_struct; // Uninitialized key

        auto [result, signature_data] =
            sign_file(temp_file.string(), private_key_struct);
        ASSERT_FALSE(result) <<
            "sign_file should fail for invalid key";
        ASSERT_TRUE(signature_data.empty()) <<
            "sign_file should fail for invalid key";
    }

    TEST_F(test_crypt, test_sign_file_empty_file) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_sign_file_empty.txt";
        FILE* f = open_file(temp_file.string(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";
        fclose(f);

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        auto [result, signature_data] =
            sign_file(temp_file.string(), private_key_struct);
        ASSERT_TRUE(result) << "Signing failed for empty file";
        ASSERT_FALSE(signature_data.empty()) << "Signing failed for empty file";
    }

    TEST_F(test_crypt, test_sign_block) {
        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> data(sample_data,
            sample_data + strlen(sample_data));

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        auto [result, signature_data] =
            sign_block(data, private_key_struct);
        ASSERT_TRUE(result) << "Signing failed for block";
        ASSERT_FALSE(signature_data.empty()) << "Signing failed for block";
    }

    TEST_F(test_crypt, test_sign_block_invalid_key) {
        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> data(sample_data,
            sample_data + strlen(sample_data));

        R_RSA_PRIVATE_KEY private_key_struct; // Uninitialized key

        auto [result, signature_data] =
            sign_block(data, private_key_struct);
        ASSERT_FALSE(result) <<
            "sign_block should fail for invalid key";
        ASSERT_TRUE(signature_data.empty()) <<
            "sign_block should fail for invalid key";
    }

    TEST_F(test_crypt, test_sign_block_empty_data) {
        std::vector<uint8_t> data; // Empty data

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        auto [result, signature_data] =
            sign_block(data, private_key_struct);
        ASSERT_TRUE(result) <<
            "Signing failed for empty data";
        ASSERT_FALSE(signature_data.empty()) <<
            "Signing failed for empty data";
    }

    TEST_F(test_crypt, test_generate_signature) {
        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> data(sample_data,
            sample_data + strlen(sample_data));

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        auto [result, signature_hex] =
            generate_signature(sample_data, private_key_struct);
        ASSERT_TRUE(result) << "Generating signature failed";
        ASSERT_FALSE(signature_hex.empty()) << "Generating signature failed";

        std::string md5_hash = get_md5(sample_data);
        std::vector<uint8_t> md5_hash_vec(md5_hash.begin(), md5_hash.end());
        auto encrypted = encrypt(md5_hash_vec, private_key);
        auto[error, expected_signature_hex] = sprint_hex_data(encrypted);
        ASSERT_TRUE(error) << "sprint_hex_data failed";
        ASSERT_EQ(signature_hex, expected_signature_hex) <<
            "Signature hex does not match expected value";
    }

    TEST_F(test_crypt, test_generate_signature_invalid_key) {
        const char* sample_data = "Hello, World!";
        std::vector<uint8_t> data(sample_data,
            sample_data + strlen(sample_data));

        R_RSA_PRIVATE_KEY private_key_struct; // Uninitialized key

        auto [result, signature_hex] =
            generate_signature(sample_data, private_key_struct);
        ASSERT_FALSE(result) <<
            "generate_signature should fail for invalid key";
        ASSERT_TRUE(signature_hex.empty()) <<
            "generate_signature should fail for invalid key";
    }

    TEST_F(test_crypt, test_generate_signature_empty_data) {
        const char* sample_data = "";
        std::vector<uint8_t> data(sample_data,
            sample_data + strlen(sample_data));

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        auto [result, signature_hex] =
            generate_signature(sample_data, private_key_struct);
        ASSERT_TRUE(result) <<
            "Generating signature failed for empty data";
        ASSERT_FALSE(signature_hex.empty()) <<
            "Generating signature failed for empty data";

        std::string md5_hash = get_md5(sample_data);
        std::vector<uint8_t> md5_hash_vec(md5_hash.begin(), md5_hash.end());
        auto encrypted = encrypt(md5_hash_vec, private_key);
        auto[error, expected_signature_hex] = sprint_hex_data(encrypted);
        ASSERT_TRUE(error) << "sprint_hex_data failed";
        ASSERT_EQ(signature_hex, expected_signature_hex) <<
            "Signature hex does not match expected value for empty data";
    }

    TEST_F(test_crypt, test_check_file_signature) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // calculate the MD5 hash of the file and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);
        ASSERT_FALSE(encrypted.empty()) <<
            "Encryption failed for signature generation";

        auto [result, is_valid] =
            check_file_signature(hash, public_key_struct, encrypted);
        ASSERT_EQ(result, 0) << "check_file_signature failed";
        ASSERT_TRUE(is_valid) << "Signature verification failed";
    }

    TEST_F(test_crypt, test_check_file_signature_invalid_signature) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // Create an invalid signature (random data)
        std::vector<uint8_t> invalid_signature(32, 0xFF); // 32 bytes of 0xFF

        auto [result, is_valid] = check_file_signature(
                sample_data, public_key_struct, invalid_signature);
        ASSERT_NE(result, 0) << "check_file_signature failed";
        ASSERT_FALSE(is_valid) <<
            "Signature verification should fail for invalid signature";
    }

    TEST_F(test_crypt, test_check_file_signature_invalid_key) {
        const char* sample_data = "Hello, World!";

        R_RSA_PUBLIC_KEY public_key_struct; // Uninitialized key

        // Create a dummy signature (random data)
        std::vector<uint8_t> dummy_signature(32, 0xAA); // 32 bytes of 0xAA

        auto [result, is_valid] = check_file_signature(
            sample_data, public_key_struct, dummy_signature);
        ASSERT_NE(result, 0) <<
            "check_file_signature should fail for invalid key";
    }

    TEST_F(test_crypt, test_check_file_signature_empty_signature) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // Create an empty signature
        std::vector<uint8_t> empty_signature;

        auto [result, is_valid] = check_file_signature(
            sample_data, public_key_struct, empty_signature);
        ASSERT_NE(result, 0) << "check_file_signature failed";
        ASSERT_FALSE(is_valid) <<
            "Signature verification should fail for empty signature";
    }

    TEST_F(test_crypt, test_check_file_signature_empty_data) {
        const char* sample_data = "";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // calculate the MD5 hash of the empty data and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);
        ASSERT_FALSE(encrypted.empty()) <<
            "Encryption failed for signature generation";

        auto [result, is_valid] = check_file_signature(
            sample_data, public_key_struct, encrypted);
        ASSERT_EQ(result, 0) << "check_file_signature failed";
        ASSERT_FALSE(is_valid) <<
            "Signature verification failed for empty data";
    }

    TEST_F(test_crypt, test_check_file_signature_invalid_data) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // calculate the MD5 hash of the file and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);
        ASSERT_FALSE(encrypted.empty()) <<
            "Encryption failed for signature generation";

        // Modify the sample data to be invalid
        const char* invalid_sample_data = "Hello, Universe!";
        auto [result, is_valid] = check_file_signature(
            invalid_sample_data, public_key_struct, encrypted);
        ASSERT_EQ(result, 0) << "check_file_signature failed";
        ASSERT_FALSE(is_valid) <<
            "Signature verification should fail for invalid data";
    }

    TEST_F(test_crypt, test_check_file_signature2) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";
        // calculate the MD5 hash of the file and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);

        auto [decrypt_result, decrypted_data] =
            decrypt_public(public_key_struct, encrypted);
        ASSERT_TRUE(decrypt_result) << "decrypt_public failed";
        ASSERT_FALSE(decrypted_data.empty()) << "decrypt_public failed";
        ASSERT_FALSE(encrypted.empty()) <<
            "Encryption failed for signature generation";

        // convert encrypted to hex string
        auto[error, signature_hex]= sprint_hex_data(encrypted);
        ASSERT_TRUE(error) << "sprint_hex_data failed for signature";
        // convert public key to hex string
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for public key";
        bool print_result = print_public_key_hex(f, public_key_struct);
        ASSERT_TRUE(print_result) <<
            "print_public_key_hex failed for public key";
        fseek(f, 0, SEEK_SET);
        std::string public_key_hex;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f) != nullptr) {
            public_key_hex += buffer;
        }
        fclose(f);

        auto [result, is_valid] =
            check_file_signature(hash, signature_hex, public_key_hex);
        ASSERT_EQ(result, 0) << "check_file_signature2 failed";
        ASSERT_TRUE(is_valid) << "Signature verification failed";
    }

    TEST_F(test_crypt, test_check_file_signature2_invalid_signature) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // Create an invalid signature (random data)
        // 64 hex characters of 'F'
        std::string invalid_signature_hex =
            "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";

        // convert public key to hex string
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for public key";
        bool print_result = print_public_key_hex(f, public_key_struct);
        ASSERT_TRUE(print_result) <<
            "print_public_key_hex failed for public key";
        fseek(f, 0, SEEK_SET);
        std::string public_key_hex;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f) != nullptr) {
            public_key_hex += buffer;
        }
        fclose(f);

        auto [result, is_valid] = check_file_signature(
            sample_data, invalid_signature_hex, public_key_hex);
        ASSERT_NE(result, 0) <<
            "check_file_signature2 should fail for invalid signature";
        ASSERT_FALSE(is_valid) <<
            "Signature verification should fail for invalid signature";
    }

    TEST_F(test_crypt, test_check_file_signature2_invalid_key) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // calculate the MD5 hash of the file and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);

        // convert encrypted to hex string
        auto[error, signature_hex] = sprint_hex_data(encrypted);
        ASSERT_TRUE(error) << "sprint_hex_data failed for signature";

        // Create an invalid public key hex string (random data)
        // 64 hex characters of 'F'
        std::string invalid_public_key_hex =
            "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";

        auto [result, is_valid] = check_file_signature(
            sample_data, signature_hex, invalid_public_key_hex);
        ASSERT_NE(result, 0) <<
            "check_file_signature2 should fail for invalid public key";
        ASSERT_FALSE(is_valid) <<
            "Signature verification should fail for invalid public key";
    }

    TEST_F(test_crypt, test_check_file_signature2_empty_signature) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // convert public key to hex string
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for public key";
        bool print_result = print_public_key_hex(f, public_key_struct);
        ASSERT_TRUE(print_result) <<
            "print_public_key_hex failed for public key";
        fseek(f, 0, SEEK_SET);
        std::string public_key_hex;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f) != nullptr) {
            public_key_hex += buffer;
        }
        fclose(f);

        // Create an empty signature hex string
        std::string empty_signature_hex;

        auto [result, is_valid] = check_file_signature(
            sample_data, empty_signature_hex, public_key_hex);
        ASSERT_NE(result, 0) <<
            "check_file_signature2 should fail for empty signature";
        ASSERT_FALSE(is_valid) <<
            "Signature verification should fail for empty signature";
    }

    TEST_F(test_crypt, test_check_file_signature2_empty_data) {
        const char* sample_data = "";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // calculate the MD5 hash of the empty data and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);

        // convert encrypted to hex string
        auto [error, signature_hex] = sprint_hex_data(encrypted);
        ASSERT_TRUE(error) << "sprint_hex_data failed for signature";

        // convert public key to hex string
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for public key";
        bool print_result = print_public_key_hex(f, public_key_struct);
        ASSERT_TRUE(print_result) <<
            "print_public_key_hex failed for public key";
        fseek(f, 0, SEEK_SET);
        std::string public_key_hex;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f) != nullptr) {
            public_key_hex += buffer;
        }
        fclose(f);

        auto [result, is_valid] = check_file_signature(
            sample_data, signature_hex, public_key_hex);
        ASSERT_EQ(result, 0) << "check_file_signature2 failed";
        ASSERT_FALSE(is_valid) <<
            "Signature verification failed for empty data";
    }

    TEST_F(test_crypt, test_check_file_signature2_invalid_data) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // calculate the MD5 hash of the file and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);

        // convert encrypted to hex string
        auto [error, signature_hex] = sprint_hex_data(encrypted);
        ASSERT_TRUE(error) << "sprint_hex_data failed for signature";

        // convert public key to hex string
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for public key";
        bool print_result = print_public_key_hex(f, public_key_struct);
        ASSERT_TRUE(print_result) <<
            "print_public_key_hex failed for public key";
        fseek(f, 0, SEEK_SET);
        std::string public_key_hex;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f) != nullptr) {
            public_key_hex += buffer;
        }
        fclose(f);

        // Modify the sample data to be invalid
        const char* invalid_sample_data = "Hello, Universe!";
        auto [result, is_valid] = check_file_signature(
            invalid_sample_data, signature_hex, public_key_hex);
        ASSERT_EQ(result, 0) << "check_file_signature2 failed";
        ASSERT_FALSE(is_valid) <<
            "Signature verification should fail for invalid data";
    }

    TEST_F(test_crypt, test_check_string_signature) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // calculate the MD5 hash of the file and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);

        // convert encrypted to hex string
        auto[error, signature_hex] = sprint_hex_data(encrypted);
        ASSERT_TRUE(error) << "sprint_hex_data failed for signature";

        auto [result, is_valid] = check_string_signature(
            sample_data, signature_hex, public_key_struct);
        ASSERT_EQ(result, 0) << "check_string_signature failed";
        ASSERT_TRUE(is_valid) << "Signature verification failed";
    }

    TEST_F(test_crypt, test_check_string_signature_invalid_signature) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // Create an invalid signature (random data)
         // 64 hex characters of 'F'
        std::string invalid_signature_hex =
            "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";

        auto [result, is_valid] = check_string_signature(
            sample_data, invalid_signature_hex, public_key_struct);
        ASSERT_NE(result, 0) <<
            "check_string_signature should fail for invalid signature";
        ASSERT_FALSE(is_valid) <<
            "Signature verification should fail for invalid signature";
    }

    TEST_F(test_crypt, test_check_string_signature_invalid_key) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // calculate the MD5 hash of the file and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);

        // convert encrypted to hex string
        auto [error, signature_hex] = sprint_hex_data(encrypted);
        ASSERT_TRUE(error) << "sprint_hex_data failed for signature";

        R_RSA_PUBLIC_KEY invalid_public_key_struct; // Uninitialized key

        auto [result, is_valid] = check_string_signature(
            sample_data, signature_hex, invalid_public_key_struct);
        ASSERT_NE(result, 0) <<
            "check_string_signature should fail for invalid key";
        ASSERT_FALSE(is_valid) <<
            "Signature verification should fail for invalid key";
    }

    TEST_F(test_crypt, test_check_string_signature_empty_data) {
        const char* sample_data = "";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // calculate the MD5 hash of the empty data and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);

        // convert encrypted to hex string
        auto [error, signature_hex] = sprint_hex_data(encrypted);
        ASSERT_TRUE(error) << "sprint_hex_data failed for signature";

        auto [result, is_valid] = check_string_signature(
            sample_data, signature_hex, public_key_struct);
        ASSERT_EQ(result, 0) << "check_string_signature failed";
        ASSERT_TRUE(is_valid) << "Signature verification failed for empty data";
    }

    TEST_F(test_crypt, test_check_string_signature_invalid_data) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // calculate the MD5 hash of the file and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);

        // convert encrypted to hex string
        auto [error, signature_hex] = sprint_hex_data(encrypted);
        ASSERT_TRUE(error) << "sprint_hex_data failed for signature";

        // Modify the sample data to be invalid
        const char* invalid_sample_data = "Hello, Universe!";
        auto [result, is_valid] = check_string_signature(
            invalid_sample_data, signature_hex, public_key_struct);
        ASSERT_EQ(result, 0) << "check_string_signature failed";
        ASSERT_FALSE(is_valid) <<
            "Signature verification should fail for invalid data";
    }

    TEST_F(test_crypt, test_check_string_signature2) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // calculate the MD5 hash of the file and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);

        // convert encrypted to hex string
        auto [error, signature_hex] = sprint_hex_data(encrypted);
        ASSERT_TRUE(error) << "sprint_hex_data failed for signature";

        // convert public key to hex string
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for public key";
        bool print_result = print_public_key_hex(f, public_key_struct);
        ASSERT_TRUE(print_result) <<
            "print_public_key_hex failed for public key";
        fseek(f, 0, SEEK_SET);
        std::string public_key_hex;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f) != nullptr) {
            public_key_hex += buffer;
        }
        fclose(f);

        auto [result, is_valid] = check_string_signature(
            sample_data, signature_hex, public_key_hex);
        ASSERT_EQ(result, 0) << "check_string_signature2 failed";
        ASSERT_TRUE(is_valid) << "Signature verification failed";
    }

    TEST_F(test_crypt, test_check_string_signature2_invalid_signature) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // Create an invalid signature (random data)
        // 64 hex characters of 'F'
        std::string invalid_signature_hex =
            "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";

        // convert public key to hex string
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for public key";
        bool print_result = print_public_key_hex(f, public_key_struct);
        ASSERT_TRUE(print_result) <<
            "print_public_key_hex failed for public key";
        fseek(f, 0, SEEK_SET);
        std::string public_key_hex;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f) != nullptr) {
            public_key_hex += buffer;
        }
        fclose(f);

        auto [result, is_valid] = check_string_signature(
            sample_data, invalid_signature_hex, public_key_hex);
        ASSERT_NE(result, 0) <<
            "check_string_signature2 should fail for invalid signature";
        ASSERT_FALSE(is_valid) <<
            "Signature verification should fail for invalid signature";
    }

    TEST_F(test_crypt, test_check_string_signature2_invalid_key) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // calculate the MD5 hash of the file and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);

        // convert encrypted to hex string
        auto [error, signature_hex] = sprint_hex_data(encrypted);
        ASSERT_TRUE(error) << "sprint_hex_data failed for signature";

        // Create an invalid public key hex string (random data)
        // 64 hex characters of 'F'
        std::string invalid_public_key_hex =
            "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";

        auto [result, is_valid] = check_string_signature(
            sample_data, signature_hex, invalid_public_key_hex);
        ASSERT_NE(result, 0) <<
            "check_string_signature2 should fail for invalid public key";
        ASSERT_FALSE(is_valid) <<
            "Signature verification should fail for invalid public key";
    }

    TEST_F(test_crypt, test_check_string_signature2_empty_data) {
        const char* sample_data = "";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // calculate the MD5 hash of the empty data and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);

        // convert encrypted to hex string
        auto [error, signature_hex] = sprint_hex_data(encrypted);
        ASSERT_TRUE(error) << "sprint_hex_data failed for signature";

        // convert public key to hex string
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for public key";
        bool print_result = print_public_key_hex(f, public_key_struct);
        ASSERT_TRUE(print_result) <<
            "print_public_key_hex failed for public key";
        fseek(f, 0, SEEK_SET);
        std::string public_key_hex;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f) != nullptr) {
            public_key_hex += buffer;
        }
        fclose(f);

        auto [result, is_valid] = check_string_signature(
            sample_data, signature_hex, public_key_hex);
        ASSERT_EQ(result, 0) << "check_string_signature2 failed";
        ASSERT_TRUE(is_valid) << "Signature verification failed for empty data";
    }

    TEST_F(test_crypt, test_check_string_signature2_invalid_data) {
        const char* sample_data = "Hello, World!";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // calculate the MD5 hash of the file and compare with signature
        std::string hash = get_md5(sample_data);
        // convert file_hash to vector<uint8_t>
        std::vector<uint8_t> hash_vec(hash.begin(), hash.end());
        auto encrypted = encrypt(hash_vec, private_key);
        // convert encrypted to hex string
        auto [error, signature_hex] = sprint_hex_data(encrypted);
        ASSERT_TRUE(error) << "sprint_hex_data failed for signature";

        // convert public key to hex string
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for public key";
        bool print_result = print_public_key_hex(f, public_key_struct);
        ASSERT_TRUE(print_result) <<
            "print_public_key_hex failed for public key";
        fseek(f, 0, SEEK_SET);
        std::string public_key_hex;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f) != nullptr) {
            public_key_hex += buffer;
        }
        fclose(f);

        // Modify the sample data to be invalid
        const char* invalid_sample_data = "Hello, Universe!";
        auto [result, is_valid] = check_string_signature(
            invalid_sample_data, signature_hex, public_key_hex);
        ASSERT_EQ(result, 0) << "check_string_signature2 failed";
        ASSERT_FALSE(is_valid) <<
            "Signature verification should fail for invalid data";
    }

    TEST_F(test_crypt, test_read_key_file) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_read_key_file.txt";
        FILE* f = open_file(temp_file.string(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        EVP_PKEY* private_key = generate_rsa_key();
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";
        std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>
            private_key_guard(private_key, EVP_PKEY_free);

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key, private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        bool print_result = print_private_key_hex(f, private_key_struct);
        fclose(f);
        ASSERT_TRUE(print_result) << "print_private_key_hex failed";

        auto[result, scanned_key_struct] = read_key_file(temp_file.string());
        ASSERT_EQ(result, 0) << "read_key_file failed";

        ASSERT_EQ(memcmp(&private_key_struct, &scanned_key_struct,
            sizeof(private_key_struct)), 0) <<
            "Key data mismatch";
    }

    TEST_F(test_crypt, test_read_key_file_invalid_path) {
        auto [result, scanned_key_struct] =
            read_key_file("non_existent_file.txt");
        ASSERT_NE(result, 0) <<
            "read_key_file should fail for non-existent file";
    }

    TEST_F(test_crypt, test_read_key_file_invalid_content) {
        std::filesystem::path temp_file =
            test_data_dir / "temp_invalid_key_file.txt";
        FILE* f = open_file(temp_file.string(), "w");
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for writing";

        // Write invalid content to the file
        fprintf(f, "Invalid key content\n");
        fclose(f);

        auto [result, scanned_key_struct] = read_key_file(temp_file.string());
        ASSERT_NE(result, 0) << "read_key_file should fail for invalid content";
    }

    TEST_F(test_crypt, test_openssl_to_public) {
        auto private_key = unique_EVP_PKEY(generate_rsa_key());
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key.get(), private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        auto [result, converted_public_key_struct] =
            openssl_to_public(private_key);
        ASSERT_EQ(result, 0) << "openssl_to_public failed";
        ASSERT_EQ(memcmp(&converted_public_key_struct, &public_key_struct,
            sizeof(public_key_struct)), 0) <<
            "Public key data mismatch";
    }

    TEST_F(test_crypt, test_openssl_to_public_invalid_key) {
        EVP_PKEY* invalid_key = nullptr; // Invalid key

        auto [result, converted_public_key_struct] =
            openssl_to_public(unique_EVP_PKEY(invalid_key));
        ASSERT_NE(result, 0) << "openssl_to_public should fail for invalid key";
    }

    TEST_F(test_crypt, test_openssl_to_private) {
        auto private_key = unique_EVP_PKEY(generate_rsa_key());
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key.get(), private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        auto [result, converted_private_key_struct] =
            openssl_to_private(private_key);
        ASSERT_EQ(result, 0) << "openssl_to_private failed";
        ASSERT_EQ(memcmp(&converted_private_key_struct, &private_key_struct,
            sizeof(private_key_struct)), 0) <<
            "Private key data mismatch";
    }

    TEST_F(test_crypt, test_openssl_to_private_invalid_key) {
        EVP_PKEY* invalid_key = nullptr; // Invalid key

        auto [result, converted_private_key_struct] =
            openssl_to_private(unique_EVP_PKEY(invalid_key));
        ASSERT_NE(result, 0) <<
            "openssl_to_private should fail for invalid key";
    }

    TEST_F(test_crypt, test_openssl_to_keys) {
        auto private_key = unique_EVP_PKEY(generate_rsa_key());
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key.get(), private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        auto [
            result,
            converted_private_key_struct,
            converted_public_key_struct
        ] = openssl_to_keys(private_key);
        ASSERT_EQ(result, 0) << "openssl_to_keys failed";
        ASSERT_EQ(memcmp(&converted_private_key_struct, &private_key_struct,
            sizeof(private_key_struct)), 0) <<
            "Private key data mismatch";
        ASSERT_EQ(memcmp(&converted_public_key_struct, &public_key_struct,
            sizeof(public_key_struct)), 0) <<
            "Public key data mismatch";
    }

    TEST_F(test_crypt, test_openssl_to_keys_invalid_key) {
        EVP_PKEY* invalid_key = nullptr; // Invalid key

        auto [result,
            converted_private_key_struct,
            converted_public_key_struct
        ] = openssl_to_keys(unique_EVP_PKEY(invalid_key));
        ASSERT_NE(result, 0) << "openssl_to_keys should fail for invalid key";
    }

    TEST_F(test_crypt, test_scan_public_key_hex) {
        auto private_key = unique_EVP_PKEY(generate_rsa_key());
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key.get(), private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // convert public key to hex string
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for public key";
        bool print_result = print_public_key_hex(f, public_key_struct);
        ASSERT_TRUE(print_result) <<
            "print_public_key_hex failed for public key";

        fseek(f, 0, SEEK_SET);
        auto [result, scanned_public_key_struct] = scan_public_key_hex(f);
        fclose(f);

        ASSERT_TRUE(result) << "scan_public_key_hex failed";
        ASSERT_EQ(memcmp(&scanned_public_key_struct, &public_key_struct,
            sizeof(public_key_struct)), 0) <<
            "Scanned public key data mismatch";
    }

    TEST_F(test_crypt, test_scan_public_key_hex_invalid_file) {
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for public key";

        // Write invalid content to the file
        fprintf(f, "Invalid key content\n");
        fseek(f, 0, SEEK_SET);

        auto [result, scanned_public_key_struct] = scan_public_key_hex(f);
        fclose(f);

        ASSERT_FALSE(result) <<
            "scan_public_key_hex should fail for invalid content";
    }

    TEST_F(test_crypt, test_scan_public_key_hex_empty_file) {
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for public key";

        // Do not write anything to the file (empty file)
        fseek(f, 0, SEEK_SET);

        auto [result, scanned_public_key_struct] = scan_public_key_hex(f);
        fclose(f);

        ASSERT_FALSE(result) <<
            "scan_public_key_hex should fail for empty file";
    }

    TEST_F(test_crypt, test_scan_public_key_hex_invalid_content) {
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for public key";

        // Write invalid content to the file
        fprintf(f, "Invalid key content\n");
        fseek(f, 0, SEEK_SET);

        auto [result, scanned_public_key_struct] = scan_public_key_hex(f);
        fclose(f);

        ASSERT_FALSE(result) <<
            "scan_public_key_hex should fail for invalid content";
    }

    TEST_F(test_crypt, test_scan_private_key_hex) {
        auto private_key = unique_EVP_PKEY(generate_rsa_key());
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key.get(), private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        // convert private key to hex string
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for private key";
        bool print_result = print_private_key_hex(f, private_key_struct);
        ASSERT_TRUE(print_result) <<
            "print_private_key_hex failed for private key";
        fseek(f, 0, SEEK_SET);

        auto [result, scanned_private_key_struct] = scan_private_key_hex(f);
        fclose(f);
        ASSERT_TRUE(result) << "scan_private_key_hex failed";
        ASSERT_EQ(memcmp(&scanned_private_key_struct, &private_key_struct,
            sizeof(private_key_struct)), 0) <<
            "Scanned private key data mismatch";
    }

    TEST_F(test_crypt, test_scan_private_key_hex_invalid_file) {
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for private key";

        // Write invalid content to the file
        fprintf(f, "Invalid key content\n");
        fseek(f, 0, SEEK_SET);

        auto [result, scanned_private_key_struct] = scan_private_key_hex(f);
        fclose(f);

        ASSERT_FALSE(result) <<
            "scan_private_key_hex should fail for invalid content";
    }

    TEST_F(test_crypt, test_scan_private_key_hex_empty_file) {
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for private key";

        // Do not write anything to the file (empty file)
        fseek(f, 0, SEEK_SET);

        auto [result, scanned_private_key_struct] = scan_private_key_hex(f);
        fclose(f);

        ASSERT_FALSE(result) <<
            "scan_private_key_hex should fail for empty file";
    }

    TEST_F(test_crypt, test_scan_private_key_hex_invalid_content) {
        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for private key";

        // Write invalid content to the file
        fprintf(f, "Invalid key content\n");
        fseek(f, 0, SEEK_SET);

        auto [result, scanned_private_key_struct] = scan_private_key_hex(f);
        fclose(f);

        ASSERT_FALSE(result) <<
            "scan_private_key_hex should fail for invalid content";
    }

    TEST_F(test_crypt, test_print_public_key_hex) {
        auto private_key = unique_EVP_PKEY(generate_rsa_key());
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key.get(), private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for public key";
        bool print_result = print_public_key_hex(f, public_key_struct);
        ASSERT_TRUE(print_result) << "print_public_key_hex failed";
        fseek(f, 0, SEEK_SET);

        // Read back the printed public key and compare with original
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";
        auto [result, scanned_public_key_struct] = scan_public_key_hex(f);
        fclose(f);
        ASSERT_TRUE(result) << "scan_public_key_hex failed";
        ASSERT_EQ(memcmp(&scanned_public_key_struct, &public_key_struct,
            sizeof(public_key_struct)), 0) <<
            "Scanned public key data mismatch";
    }

    TEST_F(test_crypt, test_print_public_key_null_file) {
        auto private_key = unique_EVP_PKEY(generate_rsa_key());
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key.get(), private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";
        bool print_result = print_public_key_hex(nullptr, public_key_struct);
        ASSERT_FALSE(print_result) <<
            "print_public_key_hex should fail for null file pointer";
    }

    TEST_F(test_crypt, test_print_private_key_hex) {
        auto private_key = unique_EVP_PKEY(generate_rsa_key());
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key.get(), private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";

        FILE* f = open_tmpfile();
        ASSERT_NE(f, nullptr) <<
            "Failed to create temporary file for private key";
        bool print_result = print_private_key_hex(f, private_key_struct);
        ASSERT_TRUE(print_result) << "print_private_key_hex failed";
        fseek(f, 0, SEEK_SET);

        // Read back the printed private key and compare with original
        ASSERT_NE(f, nullptr) << "Failed to open temporary file for reading";
        auto [result, scanned_private_key_struct] = scan_private_key_hex(f);
        fclose(f);
        ASSERT_TRUE(result) << "scan_private_key_hex failed";
        ASSERT_EQ(memcmp(&scanned_private_key_struct, &private_key_struct,
            sizeof(private_key_struct)), 0) <<
            "Scanned private key data mismatch";
    }

    TEST_F(test_crypt, test_print_private_key_null_file) {
        auto private_key = unique_EVP_PKEY(generate_rsa_key());
        ASSERT_NE(private_key, nullptr) << "Failed to generate RSA key";

        R_RSA_PRIVATE_KEY private_key_struct;
        R_RSA_PUBLIC_KEY public_key_struct;
        ASSERT_TRUE(fill_keys_from_evp(
            private_key.get(), private_key_struct, public_key_struct))
            << "Failed to fill keys from EVP_PKEY";
        bool print_result = print_private_key_hex(nullptr, private_key_struct);
        ASSERT_FALSE(print_result) <<
            "print_private_key_hex should fail for null file pointer";
    }

    TEST_F(test_crypt, test_check_validity_success) {
        auto ca_key = unique_EVP_PKEY(generate_rsa_key());
        auto leaf_key = unique_EVP_PKEY(generate_rsa_key());
        ASSERT_NE(ca_key, nullptr);
        ASSERT_NE(leaf_key, nullptr);

        std::filesystem::path ca_dir = test_data_dir / "ca_validity";
        std::filesystem::create_directories(ca_dir);

        X509* ca_cert = X509_new();
        ASSERT_NE(ca_cert, nullptr);
        std::unique_ptr<X509, decltype(&X509_free)>
            ca_cert_guard(ca_cert, X509_free);
        X509_set_version(ca_cert, 2);
        ASN1_INTEGER_set(X509_get_serialNumber(ca_cert), 1);
        X509_NAME* ca_name = X509_get_subject_name(ca_cert);
        ASSERT_NE(ca_name, nullptr);
        X509_NAME_add_entry_by_txt(ca_name, "CN", MBSTRING_ASC,
            reinterpret_cast<const unsigned char*>("Test CA"), -1, -1, 0);
        ASSERT_EQ(X509_set_issuer_name(ca_cert, ca_name), 1);
        ASSERT_EQ(X509_set_pubkey(ca_cert, ca_key.get()), 1);
        X509_gmtime_adj(X509_get_notBefore(ca_cert), 0);
        X509_gmtime_adj(X509_get_notAfter(ca_cert), 365 * 24 * 60 * 60);
        X509_EXTENSION* ca_bc = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_basic_constraints, "critical,CA:TRUE");
        ASSERT_NE(ca_bc, nullptr);
        ASSERT_EQ(X509_add_ext(ca_cert, ca_bc, -1), 1);
        X509_EXTENSION_free(ca_bc);
        X509_EXTENSION* ca_ku = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_key_usage, "critical,keyCertSign,cRLSign");
        ASSERT_NE(ca_ku, nullptr);
        ASSERT_EQ(X509_add_ext(ca_cert, ca_ku, -1), 1);
        X509_EXTENSION_free(ca_ku);
        ASSERT_GE(X509_sign(ca_cert, ca_key.get(), EVP_sha256()), 1);

        std::filesystem::path ca_cert_path = ca_dir / "ca_cert.pem";
        {
            FILE* f = open_file(ca_cert_path.string(), "wb");
            ASSERT_NE(f, nullptr);
            unique_FILE guard(f);
            ASSERT_EQ(PEM_write_X509(f, ca_cert), 1);
        }
        X509* loaded_ca = nullptr;
        {
            FILE* f = open_file(ca_cert_path.string(), "rb");
            ASSERT_NE(f, nullptr);
            unique_FILE guard(f);
            loaded_ca = PEM_read_X509(f, nullptr, nullptr, nullptr);
        }
        ASSERT_NE(loaded_ca, nullptr);
        unsigned long ca_hash = X509_subject_name_hash(loaded_ca);
        X509_free(loaded_ca);
        char ca_hash_name[16];
        snprintf(ca_hash_name, sizeof(ca_hash_name), "%08lx", ca_hash);
        std::filesystem::create_symlink(ca_cert_path,
            ca_dir / (std::string(ca_hash_name) + ".0"));

        X509* leaf_cert = X509_new();
        ASSERT_NE(leaf_cert, nullptr);
        std::unique_ptr<X509, decltype(&X509_free)>
            leaf_cert_guard(leaf_cert, X509_free);
        X509_set_version(leaf_cert, 2);
        ASN1_INTEGER_set(X509_get_serialNumber(leaf_cert), 2);
        X509_NAME* leaf_name = X509_get_subject_name(leaf_cert);
        ASSERT_NE(leaf_name, nullptr);
        X509_NAME_add_entry_by_txt(leaf_name, "CN", MBSTRING_ASC,
            reinterpret_cast<const unsigned char*>("Test Leaf"), -1, -1, 0);
        ASSERT_EQ(X509_set_issuer_name(leaf_cert, ca_name), 1);
        ASSERT_EQ(X509_set_pubkey(leaf_cert, leaf_key.get()), 1);
        X509_gmtime_adj(X509_get_notBefore(leaf_cert), 0);
        X509_gmtime_adj(X509_get_notAfter(leaf_cert), 365 * 24 * 60 * 60);
        X509_EXTENSION* leaf_bc = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_basic_constraints, "critical,CA:FALSE");
        ASSERT_NE(leaf_bc, nullptr);
        ASSERT_EQ(X509_add_ext(leaf_cert, leaf_bc, -1), 1);
        X509_EXTENSION_free(leaf_bc);
        X509_EXTENSION* leaf_ku = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_key_usage, "critical,digitalSignature,keyEncipherment");
        ASSERT_NE(leaf_ku, nullptr);
        ASSERT_EQ(X509_add_ext(leaf_cert, leaf_ku, -1), 1);
        X509_EXTENSION_free(leaf_ku);
        ASSERT_GE(X509_sign(leaf_cert, ca_key.get(), EVP_sha256()), 1);

        std::filesystem::path leaf_cert_path = test_data_dir / "leaf_valid.pem";
        {
            FILE* f = open_file(leaf_cert_path.string(), "wb");
            ASSERT_NE(f, nullptr);
            unique_FILE guard(f);
            ASSERT_EQ(PEM_write_X509(f, leaf_cert), 1);
        }

        R_RSA_PRIVATE_KEY leaf_private_key;
        R_RSA_PUBLIC_KEY leaf_public_key;
        ASSERT_TRUE(fill_keys_from_evp(leaf_key.get(), leaf_private_key,
            leaf_public_key));

        std::filesystem::path orig_file = test_data_dir / "orig_valid.txt";
        {
            std::ofstream out(orig_file);
            out << "test message";
            out.close();
        }

        auto [result, signature] = sign_file(orig_file.string(),
            leaf_private_key);
        ASSERT_TRUE(result);
        ASSERT_FALSE(signature.empty());

        std::string cert;
        std::tie(result, cert) = check_validity(test_data_dir.string(),
            orig_file.string(), signature, ca_dir.string());
        ASSERT_TRUE(result) << "check_validity failed";
        ASSERT_FALSE(cert.empty()) << "check_validity failed";
    }

    TEST_F(test_crypt, test_check_validity_missing_leaf_certificate) {
        auto ca_key = unique_EVP_PKEY(generate_rsa_key());
        auto leaf_key = unique_EVP_PKEY(generate_rsa_key());
        ASSERT_NE(ca_key, nullptr);
        ASSERT_NE(leaf_key, nullptr);

        std::filesystem::path ca_dir = test_data_dir / "ca_validity";
        std::filesystem::create_directories(ca_dir);

        X509* ca_cert = X509_new();
        ASSERT_NE(ca_cert, nullptr);
        std::unique_ptr<X509, decltype(&X509_free)>
            ca_cert_guard(ca_cert, X509_free);
        X509_set_version(ca_cert, 2);
        ASN1_INTEGER_set(X509_get_serialNumber(ca_cert), 1);
        X509_NAME* ca_name = X509_get_subject_name(ca_cert);
        ASSERT_NE(ca_name, nullptr);
        X509_NAME_add_entry_by_txt(ca_name, "CN", MBSTRING_ASC,
            reinterpret_cast<const unsigned char*>("Test CA"), -1, -1, 0);
        ASSERT_EQ(X509_set_issuer_name(ca_cert, ca_name), 1);
        ASSERT_EQ(X509_set_pubkey(ca_cert, ca_key.get()), 1);
        X509_gmtime_adj(X509_get_notBefore(ca_cert), 0);
        X509_gmtime_adj(X509_get_notAfter(ca_cert), 365 * 24 * 60 * 60);
        X509_EXTENSION* ca_bc = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_basic_constraints, "critical,CA:TRUE");
        ASSERT_NE(ca_bc, nullptr);
        ASSERT_EQ(X509_add_ext(ca_cert, ca_bc, -1), 1);
        X509_EXTENSION_free(ca_bc);
        X509_EXTENSION* ca_ku = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_key_usage, "critical,keyCertSign,cRLSign");
        ASSERT_NE(ca_ku, nullptr);
        ASSERT_EQ(X509_add_ext(ca_cert, ca_ku, -1), 1);
        X509_EXTENSION_free(ca_ku);
        ASSERT_GE(X509_sign(ca_cert, ca_key.get(), EVP_sha256()), 1);

        std::filesystem::path ca_cert_path = ca_dir / "ca_cert.pem";
        {
            FILE* f = open_file(ca_cert_path.string(), "wb");
            ASSERT_NE(f, nullptr);
            unique_FILE guard(f);
            ASSERT_EQ(PEM_write_X509(f, ca_cert), 1);
        }
        X509* loaded_ca = nullptr;
        {
            FILE* f = open_file(ca_cert_path.string(), "rb");
            ASSERT_NE(f, nullptr);
            unique_FILE guard(f);
            loaded_ca = PEM_read_X509(f, nullptr, nullptr, nullptr);
        }
        ASSERT_NE(loaded_ca, nullptr);
        unsigned long ca_hash = X509_subject_name_hash(loaded_ca);
        X509_free(loaded_ca);
        char ca_hash_name[16];
        snprintf(ca_hash_name, sizeof(ca_hash_name), "%08lx", ca_hash);
        std::filesystem::create_symlink(ca_cert_path,
            ca_dir / (std::string(ca_hash_name) + ".0"));

        X509* leaf_cert = X509_new();
        ASSERT_NE(leaf_cert, nullptr);
        std::unique_ptr<X509, decltype(&X509_free)>
            leaf_cert_guard(leaf_cert, X509_free);
        X509_set_version(leaf_cert, 2);
        ASN1_INTEGER_set(X509_get_serialNumber(leaf_cert), 2);
        X509_NAME* leaf_name = X509_get_subject_name(leaf_cert);
        ASSERT_NE(leaf_name, nullptr);
        X509_NAME_add_entry_by_txt(leaf_name, "CN", MBSTRING_ASC,
            reinterpret_cast<const unsigned char*>("Test Leaf"), -1, -1, 0);
        ASSERT_EQ(X509_set_issuer_name(leaf_cert, ca_name), 1);
        ASSERT_EQ(X509_set_pubkey(leaf_cert, leaf_key.get()), 1);
        X509_gmtime_adj(X509_get_notBefore(leaf_cert), 0);
        X509_gmtime_adj(X509_get_notAfter(leaf_cert), 365 * 24 * 60 * 60);
        X509_EXTENSION* leaf_bc = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_basic_constraints, "critical,CA:FALSE");
        ASSERT_NE(leaf_bc, nullptr);
        ASSERT_EQ(X509_add_ext(leaf_cert, leaf_bc, -1), 1);
        X509_EXTENSION_free(leaf_bc);
        X509_EXTENSION* leaf_ku = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_key_usage, "critical,digitalSignature,keyEncipherment");
        ASSERT_NE(leaf_ku, nullptr);
        ASSERT_EQ(X509_add_ext(leaf_cert, leaf_ku, -1), 1);
        X509_EXTENSION_free(leaf_ku);
        ASSERT_GE(X509_sign(leaf_cert, ca_key.get(), EVP_sha256()), 1);

        R_RSA_PRIVATE_KEY leaf_private_key;
        R_RSA_PUBLIC_KEY leaf_public_key;
        ASSERT_TRUE(fill_keys_from_evp(leaf_key.get(), leaf_private_key,
            leaf_public_key));

        std::filesystem::path orig_file = test_data_dir / "orig_valid.txt";
        {
            std::ofstream out(orig_file);
            out << "test message";
            out.close();
        }

        auto [result, signature] = sign_file(orig_file.string(),
            leaf_private_key);
        ASSERT_TRUE(result);
        ASSERT_FALSE(signature.empty());

        std::string cert;
        std::tie(result, cert) = check_validity(test_data_dir.string(),
            orig_file.string(), signature, ca_dir.string());
        ASSERT_FALSE(result) << "check_validity failed";
        ASSERT_TRUE(cert.empty()) << "check_validity failed";
    }

    TEST_F(test_crypt, test_check_validity_missing_root_certificate) {
        auto ca_key = unique_EVP_PKEY(generate_rsa_key());
        auto leaf_key = unique_EVP_PKEY(generate_rsa_key());
        ASSERT_NE(ca_key, nullptr);
        ASSERT_NE(leaf_key, nullptr);

        std::filesystem::path ca_dir = test_data_dir / "ca_validity";
        std::filesystem::create_directories(ca_dir);

        X509* ca_cert = X509_new();
        ASSERT_NE(ca_cert, nullptr);
        std::unique_ptr<X509, decltype(&X509_free)>
            ca_cert_guard(ca_cert, X509_free);
        X509_set_version(ca_cert, 2);
        ASN1_INTEGER_set(X509_get_serialNumber(ca_cert), 1);
        X509_NAME* ca_name = X509_get_subject_name(ca_cert);
        ASSERT_NE(ca_name, nullptr);
        X509_NAME_add_entry_by_txt(ca_name, "CN", MBSTRING_ASC,
            reinterpret_cast<const unsigned char*>("Test CA"), -1, -1, 0);
        ASSERT_EQ(X509_set_issuer_name(ca_cert, ca_name), 1);
        ASSERT_EQ(X509_set_pubkey(ca_cert, ca_key.get()), 1);
        X509_gmtime_adj(X509_get_notBefore(ca_cert), 0);
        X509_gmtime_adj(X509_get_notAfter(ca_cert), 365 * 24 * 60 * 60);
        X509_EXTENSION* ca_bc = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_basic_constraints, "critical,CA:TRUE");
        ASSERT_NE(ca_bc, nullptr);
        ASSERT_EQ(X509_add_ext(ca_cert, ca_bc, -1), 1);
        X509_EXTENSION_free(ca_bc);
        X509_EXTENSION* ca_ku = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_key_usage, "critical,keyCertSign,cRLSign");
        ASSERT_NE(ca_ku, nullptr);
        ASSERT_EQ(X509_add_ext(ca_cert, ca_ku, -1), 1);
        X509_EXTENSION_free(ca_ku);
        ASSERT_GE(X509_sign(ca_cert, ca_key.get(), EVP_sha256()), 1);

        X509* leaf_cert = X509_new();
        ASSERT_NE(leaf_cert, nullptr);
        std::unique_ptr<X509, decltype(&X509_free)>
            leaf_cert_guard(leaf_cert, X509_free);
        X509_set_version(leaf_cert, 2);
        ASN1_INTEGER_set(X509_get_serialNumber(leaf_cert), 2);
        X509_NAME* leaf_name = X509_get_subject_name(leaf_cert);
        ASSERT_NE(leaf_name, nullptr);
        X509_NAME_add_entry_by_txt(leaf_name, "CN", MBSTRING_ASC,
            reinterpret_cast<const unsigned char*>("Test Leaf"), -1, -1, 0);
        ASSERT_EQ(X509_set_issuer_name(leaf_cert, ca_name), 1);
        ASSERT_EQ(X509_set_pubkey(leaf_cert, leaf_key.get()), 1);
        X509_gmtime_adj(X509_get_notBefore(leaf_cert), 0);
        X509_gmtime_adj(X509_get_notAfter(leaf_cert), 365 * 24 * 60 * 60);
        X509_EXTENSION* leaf_bc = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_basic_constraints, "critical,CA:FALSE");
        ASSERT_NE(leaf_bc, nullptr);
        ASSERT_EQ(X509_add_ext(leaf_cert, leaf_bc, -1), 1);
        X509_EXTENSION_free(leaf_bc);
        X509_EXTENSION* leaf_ku = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_key_usage, "critical,digitalSignature,keyEncipherment");
        ASSERT_NE(leaf_ku, nullptr);
        ASSERT_EQ(X509_add_ext(leaf_cert, leaf_ku, -1), 1);
        X509_EXTENSION_free(leaf_ku);
        ASSERT_GE(X509_sign(leaf_cert, ca_key.get(), EVP_sha256()), 1);

        std::filesystem::path leaf_cert_path = test_data_dir / "leaf_valid.pem";
        {
            FILE* f = open_file(leaf_cert_path.string(), "wb");
            ASSERT_NE(f, nullptr);
            unique_FILE guard(f);
            ASSERT_EQ(PEM_write_X509(f, leaf_cert), 1);
        }

        R_RSA_PRIVATE_KEY leaf_private_key;
        R_RSA_PUBLIC_KEY leaf_public_key;
        ASSERT_TRUE(fill_keys_from_evp(leaf_key.get(), leaf_private_key,
            leaf_public_key));

        std::filesystem::path orig_file = test_data_dir / "orig_valid.txt";
        {
            std::ofstream out(orig_file);
            out << "test message";
            out.close();
        }

        auto [result, signature] = sign_file(orig_file.string(),
            leaf_private_key);
        ASSERT_TRUE(result);
        ASSERT_FALSE(signature.empty());

        std::string cert;
        std::tie(result, cert) = check_validity(test_data_dir.string(),
            orig_file.string(), signature, ca_dir.string());
        ASSERT_FALSE(result) << "check_validity failed";
        ASSERT_TRUE(cert.empty()) << "check_validity failed";
    }

    TEST_F(test_crypt, test_check_validity_wrong_signature) {
        auto ca_key = unique_EVP_PKEY(generate_rsa_key());
        auto leaf_key = unique_EVP_PKEY(generate_rsa_key());
        ASSERT_NE(ca_key, nullptr);
        ASSERT_NE(leaf_key, nullptr);

        std::filesystem::path ca_dir = test_data_dir / "ca_validity";
        std::filesystem::create_directories(ca_dir);

        X509* ca_cert = X509_new();
        ASSERT_NE(ca_cert, nullptr);
        std::unique_ptr<X509, decltype(&X509_free)>
            ca_cert_guard(ca_cert, X509_free);
        X509_set_version(ca_cert, 2);
        ASN1_INTEGER_set(X509_get_serialNumber(ca_cert), 1);
        X509_NAME* ca_name = X509_get_subject_name(ca_cert);
        ASSERT_NE(ca_name, nullptr);
        X509_NAME_add_entry_by_txt(ca_name, "CN", MBSTRING_ASC,
            reinterpret_cast<const unsigned char*>("Test CA"), -1, -1, 0);
        ASSERT_EQ(X509_set_issuer_name(ca_cert, ca_name), 1);
        ASSERT_EQ(X509_set_pubkey(ca_cert, ca_key.get()), 1);
        X509_gmtime_adj(X509_get_notBefore(ca_cert), 0);
        X509_gmtime_adj(X509_get_notAfter(ca_cert), 365 * 24 * 60 * 60);
        X509_EXTENSION* ca_bc = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_basic_constraints, "critical,CA:TRUE");
        ASSERT_NE(ca_bc, nullptr);
        ASSERT_EQ(X509_add_ext(ca_cert, ca_bc, -1), 1);
        X509_EXTENSION_free(ca_bc);
        X509_EXTENSION* ca_ku = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_key_usage, "critical,keyCertSign,cRLSign");
        ASSERT_NE(ca_ku, nullptr);
        ASSERT_EQ(X509_add_ext(ca_cert, ca_ku, -1), 1);
        X509_EXTENSION_free(ca_ku);
        ASSERT_GE(X509_sign(ca_cert, ca_key.get(), EVP_sha256()), 1);

        std::filesystem::path ca_cert_path = ca_dir / "ca_cert.pem";
        {
            FILE* f = open_file(ca_cert_path.string(), "wb");
            ASSERT_NE(f, nullptr);
            unique_FILE guard(f);
            ASSERT_EQ(PEM_write_X509(f, ca_cert), 1);
        }
        X509* loaded_ca = nullptr;
        {
            FILE* f = open_file(ca_cert_path.string(), "rb");
            ASSERT_NE(f, nullptr);
            unique_FILE guard(f);
            loaded_ca = PEM_read_X509(f, nullptr, nullptr, nullptr);
        }
        ASSERT_NE(loaded_ca, nullptr);
        unsigned long ca_hash = X509_subject_name_hash(loaded_ca);
        X509_free(loaded_ca);
        char ca_hash_name[16];
        snprintf(ca_hash_name, sizeof(ca_hash_name), "%08lx", ca_hash);
        std::filesystem::create_symlink(ca_cert_path,
            ca_dir / (std::string(ca_hash_name) + ".0"));

        X509* leaf_cert = X509_new();
        ASSERT_NE(leaf_cert, nullptr);
        std::unique_ptr<X509, decltype(&X509_free)>
            leaf_cert_guard(leaf_cert, X509_free);
        X509_set_version(leaf_cert, 2);
        ASN1_INTEGER_set(X509_get_serialNumber(leaf_cert), 2);
        X509_NAME* leaf_name = X509_get_subject_name(leaf_cert);
        ASSERT_NE(leaf_name, nullptr);
        X509_NAME_add_entry_by_txt(leaf_name, "CN", MBSTRING_ASC,
            reinterpret_cast<const unsigned char*>("Test Leaf"), -1, -1, 0);
        ASSERT_EQ(X509_set_issuer_name(leaf_cert, ca_name), 1);
        ASSERT_EQ(X509_set_pubkey(leaf_cert, leaf_key.get()), 1);
        X509_gmtime_adj(X509_get_notBefore(leaf_cert), 0);
        X509_gmtime_adj(X509_get_notAfter(leaf_cert), 365 * 24 * 60 * 60);
        X509_EXTENSION* leaf_bc = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_basic_constraints, "critical,CA:FALSE");
        ASSERT_NE(leaf_bc, nullptr);
        ASSERT_EQ(X509_add_ext(leaf_cert, leaf_bc, -1), 1);
        X509_EXTENSION_free(leaf_bc);
        X509_EXTENSION* leaf_ku = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_key_usage, "critical,digitalSignature,keyEncipherment");
        ASSERT_NE(leaf_ku, nullptr);
        ASSERT_EQ(X509_add_ext(leaf_cert, leaf_ku, -1), 1);
        X509_EXTENSION_free(leaf_ku);
        ASSERT_GE(X509_sign(leaf_cert, ca_key.get(), EVP_sha256()), 1);

        std::filesystem::path leaf_cert_path = test_data_dir / "leaf_valid.pem";
        {
            FILE* f = open_file(leaf_cert_path.string(), "wb");
            ASSERT_NE(f, nullptr);
            unique_FILE guard(f);
            ASSERT_EQ(PEM_write_X509(f, leaf_cert), 1);
        }

        R_RSA_PRIVATE_KEY leaf_private_key;
        R_RSA_PUBLIC_KEY leaf_public_key;
        ASSERT_TRUE(fill_keys_from_evp(leaf_key.get(), leaf_private_key,
            leaf_public_key));

        std::filesystem::path orig_file = test_data_dir / "orig_valid.txt";
        {
            std::ofstream out(orig_file);
            out << "test message";
            out.close();
        }

        std::filesystem::path wrong_file = test_data_dir / "wrong_file.txt";
        {
            std::ofstream out(wrong_file);
            out << "invalid message";
            out.close();
        }

        auto [result, signature] = sign_file(wrong_file.string(),
            leaf_private_key);
        ASSERT_TRUE(result);
        ASSERT_FALSE(signature.empty());

        std::string cert;
        std::tie(result, cert) = check_validity(test_data_dir.string(),
            orig_file.string(), signature, ca_dir.string());
        ASSERT_FALSE(result) << "check_validity failed";
        ASSERT_TRUE(cert.empty()) << "check_validity failed";
    }

    TEST_F(test_crypt, test_cert_verify_file_success) {
        auto ca_key = unique_EVP_PKEY(generate_rsa_key());
        auto leaf_key = unique_EVP_PKEY(generate_rsa_key());
        ASSERT_NE(ca_key, nullptr);
        ASSERT_NE(leaf_key, nullptr);

        std::filesystem::path trust_dir = test_data_dir / "trust";
        std::filesystem::create_directories(trust_dir);

        X509* ca_cert = X509_new();
        ASSERT_NE(ca_cert, nullptr);
        std::unique_ptr<X509, decltype(&X509_free)>
            ca_cert_guard(ca_cert, X509_free);
        X509_set_version(ca_cert, 2);
        ASN1_INTEGER_set(X509_get_serialNumber(ca_cert), 1);
        X509_NAME* ca_name = X509_get_subject_name(ca_cert);
        ASSERT_NE(ca_name, nullptr);
        X509_NAME_add_entry_by_txt(ca_name, "CN", MBSTRING_ASC,
            reinterpret_cast<const unsigned char*>("Test CA"), -1, -1, 0);
        ASSERT_EQ(X509_set_issuer_name(ca_cert, ca_name), 1);
        ASSERT_EQ(X509_set_pubkey(ca_cert, ca_key.get()), 1);
        X509_gmtime_adj(X509_get_notBefore(ca_cert), 0);
        X509_gmtime_adj(X509_get_notAfter(ca_cert), 365 * 24 * 60 * 60);
        X509_EXTENSION* ca_bc = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_basic_constraints, "critical,CA:TRUE");
        ASSERT_NE(ca_bc, nullptr);
        ASSERT_EQ(X509_add_ext(ca_cert, ca_bc, -1), 1);
        X509_EXTENSION_free(ca_bc);
        X509_EXTENSION* ca_ku = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_key_usage, "critical,keyCertSign,cRLSign");
        ASSERT_NE(ca_ku, nullptr);
        ASSERT_EQ(X509_add_ext(ca_cert, ca_ku, -1), 1);
        X509_EXTENSION_free(ca_ku);
        ASSERT_GE(X509_sign(ca_cert, ca_key.get(), EVP_sha256()), 1);

        std::filesystem::path ca_cert_path = trust_dir / "ca_cert.pem";
        {
            FILE* f = open_file(ca_cert_path.string(), "wb");
            ASSERT_NE(f, nullptr);
            unique_FILE guard(f);
            ASSERT_EQ(PEM_write_X509(f, ca_cert), 1);
        }
        X509* loaded_ca = nullptr;
        {
            FILE* f = open_file(ca_cert_path.string(), "rb");
            ASSERT_NE(f, nullptr);
            unique_FILE guard(f);
            loaded_ca = PEM_read_X509(f, nullptr, nullptr, nullptr);
        }
        ASSERT_NE(loaded_ca, nullptr);
        unsigned long ca_hash = X509_subject_name_hash(loaded_ca);
        X509_free(loaded_ca);
        char ca_hash_name[16];
        snprintf(ca_hash_name, sizeof(ca_hash_name), "%08lx", ca_hash);
        std::filesystem::create_symlink(ca_cert_path,
            trust_dir / (std::string(ca_hash_name) + ".0"));

        X509* leaf_cert = X509_new();
        ASSERT_NE(leaf_cert, nullptr);
        std::unique_ptr<X509, decltype(&X509_free)>
            leaf_cert_guard(leaf_cert, X509_free);
        X509_set_version(leaf_cert, 2);
        ASN1_INTEGER_set(X509_get_serialNumber(leaf_cert), 2);
        X509_NAME* leaf_name = X509_get_subject_name(leaf_cert);
        ASSERT_NE(leaf_name, nullptr);
        X509_NAME_add_entry_by_txt(leaf_name, "CN", MBSTRING_ASC,
            reinterpret_cast<const unsigned char*>("Test Leaf"), -1, -1, 0);
        ASSERT_EQ(X509_set_issuer_name(leaf_cert, ca_name), 1);
        ASSERT_EQ(X509_set_pubkey(leaf_cert, leaf_key.get()), 1);
        X509_gmtime_adj(X509_get_notBefore(leaf_cert), 0);
        X509_gmtime_adj(X509_get_notAfter(leaf_cert), 365 * 24 * 60 * 60);
        X509_EXTENSION* leaf_bc = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_basic_constraints, "critical,CA:FALSE");
        ASSERT_NE(leaf_bc, nullptr);
        ASSERT_EQ(X509_add_ext(leaf_cert, leaf_bc, -1), 1);
        X509_EXTENSION_free(leaf_bc);
        X509_EXTENSION* leaf_ku = X509V3_EXT_conf_nid(nullptr, nullptr,
            NID_key_usage, "critical,digitalSignature,keyEncipherment");
        ASSERT_NE(leaf_ku, nullptr);
        ASSERT_EQ(X509_add_ext(leaf_cert, leaf_ku, -1), 1);
        X509_EXTENSION_free(leaf_ku);
        ASSERT_GE(X509_sign(leaf_cert, ca_key.get(), EVP_sha256()), 1);

        std::filesystem::path leaf_cert_path = trust_dir / "leaf_cert.pem";
        {
            FILE* f = open_file(leaf_cert_path.string(), "wb");
            ASSERT_NE(f, nullptr);
            unique_FILE guard(f);
            ASSERT_EQ(PEM_write_X509(f, leaf_cert), 1);
        }
        X509* loaded_leaf = nullptr;
        {
            FILE* f = open_file(leaf_cert_path.string(), "rb");
            ASSERT_NE(f, nullptr);
            unique_FILE guard(f);
            loaded_leaf = PEM_read_X509(f, nullptr, nullptr, nullptr);
        }
        ASSERT_NE(loaded_leaf, nullptr);
        unsigned long leaf_hash = X509_subject_name_hash(loaded_leaf);
        char leaf_hash_name[16];
        snprintf(leaf_hash_name, sizeof(leaf_hash_name), "%08lx", leaf_hash);
        char leaf_subject[256];
        X509_NAME_oneline(X509_get_subject_name(loaded_leaf), leaf_subject,
            sizeof(leaf_subject));
        std::filesystem::create_symlink(leaf_cert_path,
            trust_dir / (std::string(leaf_hash_name) + ".0"));

        R_RSA_PRIVATE_KEY leaf_private_key;
        R_RSA_PUBLIC_KEY leaf_public_key;
        ASSERT_TRUE(fill_keys_from_evp(leaf_key.get(), leaf_private_key,
            leaf_public_key));

        std::filesystem::path orig_file =
            test_data_dir / "orig_cert_verify.txt";
        {
            std::ofstream out(orig_file);
            out << "test message";
            out.close();
        }

        auto [result, signature] = sign_file(orig_file.string(),
            leaf_private_key);
        ASSERT_TRUE(result);
        ASSERT_FALSE(signature.empty());
        auto [error, signature_hex] = sprint_hex_data(signature);
        ASSERT_TRUE(error) << "Failed to convert signature to hex";

        CERT_SIGS cert_sigs;
        CERT_SIG sig;
        sig.clear();
        safe_strcpy(sig.signature, signature_hex.c_str());
        snprintf(sig.hash, sizeof(sig.hash), "%08lx", leaf_hash);
        safe_strcpy(sig.subject, leaf_subject);
        sig.type = MD5_HASH;
        cert_sigs.signatures.push_back(sig);

        ASSERT_TRUE(cert_verify_file(&cert_sigs, orig_file.string(),
            trust_dir.string()));
    }

    TEST_F(test_crypt, test_cert_verify_file_rejects_empty_signature_list) {
        CERT_SIGS cert_sigs;
        std::filesystem::path orig_file = test_data_dir / "orig_empty.txt";
        {
            std::ofstream out(orig_file);
            out << "test message";
            out.close();
        }
        ASSERT_FALSE(cert_verify_file(&cert_sigs, orig_file.string(),
            test_data_dir.string()));
    }
}
