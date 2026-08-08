#!/usr/bin/env php
<?php

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

// usage: generate_account_ownership_keys.php [overwrite]
//
// This script generates the public and private keys for the proof of
// account ownership feature of the website that is described
// here: https://github.com/BOINC/boinc/wiki/ProofOfOwnership
//
// If the overwrite option is set, then the existing keys will be deleted and
// new keys generated.  This will invalidate all existing account ownership
// proofs so use with caution.

require_once("../inc/boinc_db.inc");
require_once("../inc/user.inc");
require_once("../inc/util.inc");
require_once("../inc/account_ownership.inc");

if ( !function_exists('openssl_pkey_new') ) {
    echo "WARNING: OpenSSL functions not available.  Not generating account ownership keys.\n";
    exit(1);
}

if (php_sapi_name() == "cli") {
    if (!empty($argv[1])) {
        if ($argv[1] == "overwrite") {
            if (file_exists($account_ownership_private_key_file_path)) {
                // If the private key exists, delete it.
                unlink($account_ownership_private_key);
                echo "erased '$account_ownership_private_key_file_name' \n";
            }
            if (file_exists($account_ownership_public_key_file_path)) {
                // If the public key exists, delete it.
                unlink($account_ownership_public_key);
                echo "erased '$account_ownership_public_key_file_name' \n";
            }
        }
    }

    if ((!file_exists($account_ownership_private_key_file_path)) && (!file_exists($account_ownership_public_key_file_path))) {
        try {
            $generated_pkey = openssl_pkey_new(array(
                'digest_alg' => 'sha512',
                'private_key_bits' => 4096,
                'private_key_type' => OPENSSL_KEYTYPE_RSA
            ));

            $pubkey = openssl_pkey_get_details($generated_pkey); // Get the public key from the generated pkey pair
            file_put_contents($account_ownership_public_key, $pubkey['key']); // Save the public key to disk
            openssl_pkey_export_to_file($generated_pkey, $account_ownership_private_key); // Save the private key to disk
            openssl_pkey_free($generated_pkey); // Free key data securely from memory

            if ((file_exists($account_ownership_private_key_file_path)) && (file_exists($account_ownership_public_key_file_path))) {
                echo "Successfully generated a new account ownership keypair. \n";
            } else {
                throw new Exception('Failed to generate account ownership keypair.');
            }
        } catch (Exception $e) {
            echo 'Caught exception during account ownership key generation: ',  $e->getMessage(), "\n";
        }
    } else {
        echo "The private and public keys already exist. Repeat the command with the 'overwrite' parameter to replace the existing ownership keys. \n";
    }
} else {
    echo "This script must be run from the CLI \n";
}

?>
