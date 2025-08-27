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

#include "gtest/gtest.h"

#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/core_names.h>

#include "crypt.h"
#include "md5_file.h"

using std::string;
using std::vector;

namespace test_crypt {

// Utilities
static EVP_PKEY* make_evp_rsa_1024() {
	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
	if (!ctx) return nullptr;
	if (EVP_PKEY_keygen_init(ctx) <= 0) {
		EVP_PKEY_CTX_free(ctx);
		return nullptr;
	}
	if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 1024) <= 0) {
		EVP_PKEY_CTX_free(ctx);
		return nullptr;
	}
	EVP_PKEY* pkey = nullptr;
	if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
		EVP_PKEY_CTX_free(ctx);
		return nullptr;
	}
	EVP_PKEY_CTX_free(ctx);
	return pkey;
}

static void bn_to_fixed(const BIGNUM* bn, unsigned char* out, int out_len) {
	memset(out, 0, out_len);
	if (!bn) return;
	int bytes = BN_num_bytes(bn);
	if (bytes > out_len) {
		// take the least significant bytes
		std::vector<unsigned char> tmp(bytes);
		BN_bn2bin(bn, tmp.data());
		memcpy(out + (out_len - out_len), tmp.data() + (bytes - out_len), out_len);
	} else {
		BN_bn2bin(bn, out + (out_len - bytes));
	}
}

static bool fill_keys_from_evp(EVP_PKEY* pkey, int nbits, R_RSA_PRIVATE_KEY& priv, R_RSA_PUBLIC_KEY& pub) {
	BIGNUM *n=nullptr, *e=nullptr, *d=nullptr, *p=nullptr, *q=nullptr, *dmp1=nullptr, *dmq1=nullptr, *iqmp=nullptr;
	if (!EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_N, &n)) return false;
	if (!EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_E, &e)) { BN_free(n); return false; }
	// Private params are optional on public-only keys
	EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_D, &d);
	EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_FACTOR1, &p);
	EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_FACTOR2, &q);
	EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_EXPONENT1, &dmp1);
	EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_EXPONENT2, &dmq1);
	EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_COEFFICIENT, &iqmp);

	pub.bits = (unsigned short)nbits;
	bn_to_fixed(n, pub.modulus, sizeof(pub.modulus));
	bn_to_fixed(e, pub.exponent, sizeof(pub.exponent));

	memset(&priv, 0, sizeof(priv));
	priv.bits = (unsigned short)nbits;
	bn_to_fixed(n, priv.modulus, sizeof(priv.modulus));
	bn_to_fixed(e, priv.publicExponent, sizeof(priv.publicExponent));
	bn_to_fixed(d, priv.exponent, sizeof(priv.exponent));
	bn_to_fixed(p, priv.prime[0], sizeof(priv.prime[0]));
	bn_to_fixed(q, priv.prime[1], sizeof(priv.prime[1]));
	bn_to_fixed(dmp1, priv.primeExponent[0], sizeof(priv.primeExponent[0]));
	bn_to_fixed(dmq1, priv.primeExponent[1], sizeof(priv.primeExponent[1]));
	bn_to_fixed(iqmp, priv.coefficient, sizeof(priv.coefficient));

	if (n) BN_free(n);
	if (e) BN_free(e);
	if (d) BN_free(d);
	if (p) BN_free(p);
	if (q) BN_free(q);
	if (dmp1) BN_free(dmp1);
	if (dmq1) BN_free(dmq1);
	if (iqmp) BN_free(iqmp);
	return true;
}

static string key_to_string(KEY* key, int size) {
	FILE* f = tmpfile();
	if (!f) return {};
	print_key_hex(f, key, size);
	fflush(f);
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	rewind(f);
	string out;
	out.resize((size_t)sz);
	fread(&out[0], 1, (size_t)sz, f);
	fclose(f);
	return out;
}

static string data_to_hex_string(const vector<unsigned char>& bytes) {
	DATA_BLOCK db;
	db.data = const_cast<unsigned char*>(bytes.data());
	db.len = (unsigned int)bytes.size();
	// Max chars: 2 per byte + newlines each 32 bytes + ".\n" and possible extra \n
	size_t max_len = 2 * bytes.size() + (bytes.size() / 32 + 2) + 4;
	vector<char> buf(max_len + 8, 0);
	sprint_hex_data(buf.data(), db);
	return string(buf.data());
}

static vector<unsigned char> hex_string_to_data(const string& hex) {
	FILE* f = tmpfile();
	fwrite(hex.data(), 1, hex.size(), f);
	rewind(f);
	vector<unsigned char> out(1024);
	DATA_BLOCK db;
	db.data = out.data();
	db.len = (unsigned int)out.size();
	scan_hex_data(f, db);
	fclose(f);
	out.resize(db.len);
	return out;
}

// bn_equal helper removed (unused)

class CryptTest : public ::testing::Test {
protected:
	void TearDown() override {}
};

TEST_F(CryptTest, SprintAndScanHexRoundTrip) {
	// cover sprint_hex_data and scan_hex_data with >32 bytes to hit newline behavior
	vector<unsigned char> input(100);
	for (size_t i = 0; i < input.size(); i++) input[i] = (unsigned char)i;
	string hex = data_to_hex_string(input);
	auto output = hex_string_to_data(hex);
	ASSERT_EQ(output.size(), input.size());
	EXPECT_TRUE(std::equal(input.begin(), input.end(), output.begin()));
}

TEST_F(CryptTest, OpenSSLToKeysAndBack) {
	EVP_PKEY* pkey = make_evp_rsa_1024();
	ASSERT_NE(pkey, nullptr);

	R_RSA_PRIVATE_KEY priv{};
	R_RSA_PUBLIC_KEY pub{};
	const int nbits = 1024; // we generated 1024-bit key
	ASSERT_TRUE(fill_keys_from_evp(pkey, nbits, priv, pub));

	// Basic sanity on produced keys (exponent should be 65537; modulus non-zero)
	bool all_zero = true;
	for (unsigned char c : pub.modulus) if (c) { all_zero = false; break; }
	EXPECT_FALSE(all_zero);
	// publicExponent ends with 0x01 0x00 0x01 in big endian (position depends on length)
	EXPECT_EQ(pub.exponent[MAX_RSA_MODULUS_LEN-1], 0x01);
	EXPECT_EQ(pub.exponent[MAX_RSA_MODULUS_LEN-3], 0x01);
	EXPECT_EQ(pub.bits, nbits);
	EVP_PKEY_free(pkey);
}

TEST_F(CryptTest, EncryptPrivateThenDecryptPublic) {
	EVP_PKEY* pkey = make_evp_rsa_1024();
	ASSERT_NE(pkey, nullptr);

	R_RSA_PRIVATE_KEY priv{};
	R_RSA_PUBLIC_KEY pub{};
	ASSERT_TRUE(fill_keys_from_evp(pkey, 1024, priv, pub));

	const char* msg = "test message";
	DATA_BLOCK in;
	in.data = (unsigned char*)msg;
	in.len = (unsigned int)strlen(msg);

	unsigned char sigbuf[SIGNATURE_SIZE_BINARY] = {0};
	DATA_BLOCK sig;
	sig.data = sigbuf;
	sig.len = sizeof(sigbuf);
	ASSERT_EQ(encrypt_private(priv, in, sig), 0);

	unsigned char outbuf[SIGNATURE_SIZE_BINARY] = {0};
	DATA_BLOCK out;
	out.data = outbuf;
	out.len = sizeof(outbuf);
	ASSERT_EQ(decrypt_public(pub, sig, out), 0);

	// Expect the decrypted prefix to equal original message
	EXPECT_EQ(0, memcmp(outbuf, msg, in.len));

	EVP_PKEY_free(pkey);
}

TEST_F(CryptTest, GenerateAndVerifySignatureOnText) {
	EVP_PKEY* pkey = make_evp_rsa_1024();
	ASSERT_NE(pkey, nullptr);

	R_RSA_PRIVATE_KEY priv{};
	R_RSA_PUBLIC_KEY pub{};
	ASSERT_TRUE(fill_keys_from_evp(pkey, 1024, priv, pub));

	char text[] = "The quick brown fox jumps over the lazy dog";
	char sig_hex[SIGNATURE_SIZE_TEXT] = {0};
	ASSERT_EQ(generate_signature(text, sig_hex, priv), 0);

	// Build key text for the public key
	string key_text = key_to_string((KEY*)&pub, sizeof(pub));

	bool answer = false;
	ASSERT_EQ(check_string_signature2(text, sig_hex, key_text.c_str(), answer), 0);
	EXPECT_TRUE(answer);

	// Tamper 1 hex character and ensure verification fails
	string bad_sig(sig_hex);
	for (char& c : bad_sig) {
		if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) {
			// flip nibble slightly
			if (c == '0') c = '1';
			else if (c == '1') c = '0';
			else if (c == 'a') c = 'b';
			else if (c == 'b') c = 'a';
			else if (c == 'c') c = 'd';
			else if (c == 'd') c = 'c';
			else if (c == 'e') c = 'f';
			else if (c == 'f') c = 'e';
			else c = '0';
			break;
		}
	}
	answer = true; // reset to ensure it flips
	// Tampered signature should fail verification. The implementation
	// returns a non-zero error (e.g., ERR_CRYPTO) on padding failure.
	int rc = check_string_signature2(text, bad_sig.c_str(), key_text.c_str(), answer);
	EXPECT_NE(rc, 0);

	EVP_PKEY_free(pkey);
}

TEST_F(CryptTest, KeyStringParseRoundTrip) {
	EVP_PKEY* pkey = make_evp_rsa_1024();
	ASSERT_NE(pkey, nullptr);

	R_RSA_PRIVATE_KEY priv{};
	R_RSA_PUBLIC_KEY pub{};
	ASSERT_TRUE(fill_keys_from_evp(pkey, 1024, priv, pub));

	string priv_text = key_to_string((KEY*)&priv, sizeof(priv));
	string pub_text = key_to_string((KEY*)&pub, sizeof(pub));

	R_RSA_PRIVATE_KEY priv2{};
	R_RSA_PUBLIC_KEY pub2{};
	ASSERT_EQ(sscan_key_hex(priv_text.c_str(), (KEY*)&priv2, sizeof(priv2)), 0);
	ASSERT_EQ(sscan_key_hex(pub_text.c_str(), (KEY*)&pub2, sizeof(pub2)), 0);

	EXPECT_EQ(priv.bits, priv2.bits);
	EXPECT_EQ(pub.bits, pub2.bits);
	EXPECT_EQ(0, memcmp(&priv, &priv2, sizeof(priv)));
	EXPECT_EQ(0, memcmp(&pub, &pub2, sizeof(pub)));

	EVP_PKEY_free(pkey);
}

} // namespace test_crypt
