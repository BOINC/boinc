#!/usr/bin/env python3

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2026 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

import datetime
import filecmp
import os
import subprocess
import sys
import testset

from cryptography import x509
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives import hashes
from OpenSSL import crypto

class IntegrationTests:
    def __init__(self, crypt_prog):
        self.crypt_prog = crypt_prog
        self.unsupported_bits = ["512", "2048"]
        self.bits = ["1024"]
        self.result = True
        self.result &= self.test_genkey()
        self.result &= self.test_cert_store()

    def test_genkey(self):
        ts = testset.TestSet("Test genkey")
        for bit in self.unsupported_bits:
            ts.expect_false(self._genkey(bit, "private.key", "public.key"), "Test genkey with " + bit + " bits")

        for bit in self.bits:
            ts.expect_true(self._genkey(bit, "private.key", "public.key"), "Test genkey with " + bit + " bits")
            ts.expect_true(self._genkey(bit, "private1.key", "public1.key"), "Test genkey with " + bit + " bits")
            ts.expect_false(filecmp.cmp("private.key", "private1.key"), "Test two private keys are different")
            ts.expect_false(filecmp.cmp("public.key", "public1.key"), "Test two public keys are different")

            ts.expect_true(self._convkey("b2o", "priv", "private.key", "private_o.key"), "Test convert private key")
            ts.expect_true(self._convkey("o2b", "priv", "private_o.key", "private_b.key"), "Test convert private key")
            ts.expect_true(filecmp.cmp("private.key", "private_b.key"), "Test two private keys are the same")
            ts.expect_true(self._convkey("b2o", "pub", "public.key", "public_o.key"), "Test convert public key")
            ts.expect_true(self._convkey("o2b", "pub", "public_o.key", "public_b.key"), "Test convert public key")
            ts.expect_true(filecmp.cmp("public.key", "public_b.key"), "Test two public keys are the same")

            result, signature = self._sign_file(self.crypt_prog, "private.key")
            with open("signature", "wb") as f:
                f.write(signature)
            ts.expect_true(result, "Test sign file")
            ts.expect_true(self._verify_file(self.crypt_prog, "signature", "public.key"), "Test verify file")

            result, signature1 = self._sign_file(self.crypt_prog, "private1.key")
            with open("signature1", "wb") as f:
                f.write(signature1)
            ts.expect_true(result, "Test sign file")
            ts.expect_true(self._verify_file(self.crypt_prog, "signature1", "public1.key"), "Test verify file")

            ts.expect_false(filecmp.cmp("signature", "signature1"), "Test two signatures are different")
            ts.expect_false(self._verify_file(self.crypt_prog, "signature1", "public.key"), "Test verify file with wrong signature")
            ts.expect_false(self._verify_file(self.crypt_prog, "signature", "public1.key"), "Test verify file with wrong signature")

            ts.expect_true(self._convsig("b2o", "signature", "signature_o"), "Test convert signature")
            ts.expect_true(self._convsig("o2b", "signature_o", "signature_b"), "Test convert signature")
            ts.expect_true(filecmp.cmp("signature", "signature_b"), "Test two signatures are the same")
            ts.expect_true(self._convsig("b2o", "signature1", "signature1_o"), "Test convert signature")
            ts.expect_true(self._convsig("o2b", "signature1_o", "signature1_b"), "Test convert signature")
            ts.expect_true(filecmp.cmp("signature1", "signature1_b"), "Test two signatures are the same")

            result, signature = self._sign_string("test", "private.key")
            with open("signature", "wb") as f:
                f.write(signature)
            ts.expect_true(result, "Test sign string")
            ts.expect_true(self._verify_string("test", "signature", "public.key"), "Test verify string")

            result, signature1 = self._sign_string("test", "private1.key")
            with open("signature1", "wb") as f:
                f.write(signature1)
            ts.expect_true(result, "Test sign string")
            ts.expect_true(self._verify_string("test", "signature1", "public1.key"), "Test verify string")

            ts.expect_false(filecmp.cmp("signature", "signature1"), "Test two signatures are different")
            ts.expect_false(self._verify_string("test", "signature1", "public.key"), "Test verify string with wrong signature")
            ts.expect_false(self._verify_string("test", "signature", "public1.key"), "Test verify string with wrong signature")

            ts.expect_true(self._convsig("b2o", "signature", "signature_o"), "Test convert signature")
            ts.expect_true(self._convsig("o2b", "signature_o", "signature_b"), "Test convert signature")
            ts.expect_true(filecmp.cmp("signature", "signature_b"), "Test two signatures are the same")
            ts.expect_true(self._convsig("b2o", "signature1", "signature1_o"), "Test convert signature")
            ts.expect_true(self._convsig("o2b", "signature1_o", "signature1_b"), "Test convert signature")
            ts.expect_true(filecmp.cmp("signature1", "signature1_b"), "Test two signatures are the same")

            ts.expect_true(self._test_crypt("private.key", "public.key"), "Test crypt")

            self._clean_up(["private.key", "public.key", "private1.key", "public1.key", "private_o.key", "public_o.key", "private_b.key", "public_b.key",
                            "signature", "signature1", "signature_o", "signature_b", "signature1_o", "signature1_b"])
        return ts.result()

    def test_cert_store(self):
        ts = testset.TestSet("Test cert store")
        cwd = os.getcwd()
        ca_dir = os.path.join(cwd, "ca")
        for bit in self.bits:
            if os.path.exists(ca_dir):
                for file in os.listdir(ca_dir):
                    os.remove(os.path.join(ca_dir, file))
                os.rmdir(ca_dir)
            os.makedirs(ca_dir)

            ts.expect_true(self._genkey(bit, "b_ca_private.key", "b_ca_public.key"), "Test genkey with " + bit + " bits")
            ts.expect_true(self._convkey("b2o", "priv", "b_ca_private.key", "ca_private.key"), "Test convert private key")
            ts.expect_true(self._convkey("b2o", "pub", "b_ca_public.key", "ca_public.key"), "Test convert public key")
            ca_subject = x509.Name([x509.NameAttribute(x509.NameOID.COMMON_NAME, u"Test CA")])
            ca_cert = (
                x509.CertificateBuilder()
                .subject_name(ca_subject)
                .issuer_name(ca_subject)
                .public_key(serialization.load_pem_public_key(open("ca_public.key", "rb").read()))
                .serial_number(x509.random_serial_number())
                .not_valid_before(datetime.datetime.now(datetime.UTC))
                .not_valid_after(datetime.datetime.now(datetime.UTC) + datetime.timedelta(days=365))
                .add_extension(
                    x509.BasicConstraints(ca=True, path_length=None), critical=True,
                )
                .add_extension(
                    x509.KeyUsage(
                        digital_signature=False,
                        key_encipherment=False,
                        content_commitment=False,
                        data_encipherment=False,
                        key_agreement=False,
                        key_cert_sign=True,
                        crl_sign=True,
                        encipher_only=False,
                        decipher_only=False),
                    critical=True,
                )
                .sign(serialization.load_pem_private_key(open("ca_private.key", "rb").read(), password=None), hashes.SHA256())
            )
            ca_cert_path = os.path.join(ca_dir, "ca_cert.pem")
            ca_cert_bytes = ca_cert.public_bytes(serialization.Encoding.PEM)
            with open(ca_cert_path, "wb") as f:
                f.write(ca_cert_bytes)
            ca_x509_cert = crypto.load_certificate(crypto.FILETYPE_PEM, ca_cert_bytes)
            ca_hash = f"{ca_x509_cert.subject_name_hash():08x}"
            os.symlink(ca_cert_path, os.path.join(ca_dir, f"{ca_hash}.0"))

            ts.expect_true(self._genkey(bit, "b_leaf_private.key", "b_leaf_public.key"), "Test genkey with " + bit + " bits")
            ts.expect_true(self._convkey("b2o", "priv", "b_leaf_private.key", "leaf_private.key"), "Test convert private key")
            ts.expect_true(self._convkey("b2o", "pub", "b_leaf_public.key", "leaf_public.key"), "Test convert public key")
            leaf_subject = x509.Name([x509.NameAttribute(x509.NameOID.COMMON_NAME, u"Test Leaf")])
            leaf_cert = (
                x509.CertificateBuilder()
                .subject_name(leaf_subject)
                .issuer_name(ca_cert.subject)
                .public_key(serialization.load_pem_public_key(open("leaf_public.key", "rb").read()))
                .serial_number(x509.random_serial_number())
                .not_valid_before(datetime.datetime.now(datetime.UTC))
                .not_valid_after(datetime.datetime.now(datetime.UTC) + datetime.timedelta(days=365))
                .add_extension(
                    x509.BasicConstraints(ca=False, path_length=None), critical=True,
                )
                .add_extension(
                    x509.KeyUsage(
                        digital_signature=True,
                        key_encipherment=True,
                        content_commitment=False,
                        data_encipherment=False,
                        key_agreement=False,
                        key_cert_sign=False,
                        crl_sign=False,
                        encipher_only=False,
                        decipher_only=False),
                    critical=True,
                )
                .sign(serialization.load_pem_private_key(open("ca_private.key", "rb").read(), password=None), hashes.SHA256())
            )
            leaf_cert_path = os.path.join(cwd, "leaf_cert.pem")
            leaf_cert_bytes = leaf_cert.public_bytes(serialization.Encoding.PEM)
            with open(leaf_cert_path, "wb") as f:
                f.write(leaf_cert_bytes)
            leaf_x509_cert = crypto.load_certificate(crypto.FILETYPE_PEM, leaf_cert_bytes)
            leaf_hash = f"{leaf_x509_cert.subject_name_hash():08x}"
            os.symlink(leaf_cert_path, os.path.join(ca_dir, f"{leaf_hash}.0"))

            result, signature = self._sign_file(self.crypt_prog, "b_leaf_private.key")
            with open("signature", "wb") as f:
                f.write(signature)
            ts.expect_true(result, "Test sign file with leaf private key")
            ts.expect_true(self._cert_verify(self.crypt_prog, "signature", cwd, ca_dir), "Test verify file with leaf public key and CA cert")

            self._clean_up(["ca_private.key", "ca_public.key", "b_ca_private.key", "b_ca_public.key", ca_cert_path, "leaf_private.key", "leaf_public.key", "b_leaf_private.key", "b_leaf_public.key", leaf_cert_path, "signature"])
            for file in os.listdir(ca_dir):
                os.remove(os.path.join(ca_dir, file))
            os.rmdir(ca_dir)
        return ts.result()

    def _clean_up(self, files):
        for file in files:
            if os.path.exists(file):
                os.remove(file)

    def _run_crypt_prog(self, args):
        proc = subprocess.Popen((self.crypt_prog + " " + args).split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        result, err = proc.communicate()
        exit_code = proc.wait()
        return result, err, exit_code

    def _genkey(self, bits, private, public):
        _, _, exit_code = self._run_crypt_prog("-genkey " + bits + " " + private + " " + public)
        return exit_code == 0

    def _sign_file(self, file, private):
        signature, _, exit_code = self._run_crypt_prog("-sign " + file + " " + private)
        return exit_code == 0, signature

    def _sign_string(self, string, private):
        signature, _, exit_code = self._run_crypt_prog("-sign_string " + string + " " + private)
        return exit_code == 0, signature

    def _verify_file(self, file, signature, public):
        _, _, exit_code = self._run_crypt_prog("-verify " + file + " " + signature + " " + public)
        return exit_code == 0

    def _verify_string(self, string, signature, public):
        _, _, exit_code = self._run_crypt_prog("-verify_string " + string + " " + signature + " " + public)
        return exit_code == 0

    def _test_crypt(self, private, public):
        result, _, exit_code = self._run_crypt_prog("-test_crypt " + private + " " + public)
        return exit_code == 0, result

    def _cert_verify(self, file, signature, certificate_dir, ca_dir):
        _, _, exit_code = self._run_crypt_prog("-cert_verify " + file + " " + signature + " " + certificate_dir + " " + ca_dir)
        return exit_code == 0

    def _convsig(self, mode, signature_in, signature_out):
        _, _, exit_code = self._run_crypt_prog("-convsig " + mode + " " + signature_in + " " + signature_out)
        return exit_code == 0

    def _convkey(self, mode, type, key_in, key_out):
        _, _, exit_code = self._run_crypt_prog("-convkey " + mode + " " + type + " " + key_in + " " + key_out)
        return exit_code == 0

if __name__ == "__main__":
    if len(sys.argv) == 2:
        crypt_prog = sys.argv[1]
    else:
        print("Usage: python crypt_prog_tests.py <crypt_prog>")
        sys.exit(1)
    if not IntegrationTests(crypt_prog).result:
        sys.exit(1)
    sys.exit(0)
