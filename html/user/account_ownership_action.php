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

require_once("../inc/boinc_db.inc");
require_once("../inc/user.inc");
require_once("../inc/util.inc");
require_once('../inc/recaptchalib.php');

check_get_args(array("tnow", "ttok"));

// Check the user is online
$user = get_logged_in_user();
check_tokens($user->authenticator);

page_head(tra("Proof of account ownership results"), null, null, null, boinc_recaptcha_get_head_extra());

global $recaptcha_private_key;
if ($recaptcha_private_key) {
    // Recaptcha is enabled on the BOINC server
    if (!boinc_recaptcha_isValidated($recaptcha_private_key)) {
        // The user failed to solve the recaptcha prompt - redirect them to an error message!
        error_page(
            tra("Your reCAPTCHA response was not correct. Please try again.")
        );
    }
}

// Input is passed in from the openssl_sign_form
$user_data = htmlentities(post_str("user_data", true), ENT_QUOTES, "UTF-8"); // Convert special characters to html equivelant

if ((strlen($user_data) > 0) && (strlen($user_data) <= 4096)) {
    // The user data input is valid
    $config = get_config();
    $keydir = parse_config($config, "<key_dir>");

    /*
        How to generate required keys in /project/keys/ folder:
        openssl genpkey -algorithm RSA -out ownership_sign_private.pem -pkeyopt rsa_keygen_bits:2048
        openssl rsa -pubout -in ownership_sign_private.pem -out ownership_sign_public.pem
        chown -R boincadm:boincadm ext*
        chmod --reference upload_private ownership_sign_public.pem
        chmod --reference upload_private ownership_sign_private.pem
    */

    // If the following keys do not exist, then the users will be shown an error message.
    $private_key_path = "file://$keydir/ownership_sign_private.pem";
    $public_key_path = "file://$keydir/ownership_sign_public.pem";

    // Check that the private key file exists where specified. If not, redirect to error page.
    if (!file_exists($private_key_path)) {
        error_page(tra("The required private key doesn't exist. Contact the project administrator to resolve this issue."));
    }

    // Check that the public key file exists where specified. If not, redirect to error page.
    if (!file_exists($public_key_path)) {
        error_page(tra("The required public key doesn't exist. Contact the project administrator to resolve this issue."));
    }

    $privkey = fopen($private_key_path, "r"); // Opening private key file
    if (!isset($privkey) || empty($privkey)) {
      error_page(tra("Unable to access the required private key. Contact the project administrator to resolve this issue."));
    }
    $privkey_contents = fread($privkey, 8192); // Reading contents of private key into var
    fclose($privkey); // Closing private key file

    $userid = $user->id; // Retrieving the user's UserId
    $message_data = "$userid $user_data"; // Create the message which will be signed.

    $private_key_pem = openssl_pkey_get_private($privkey_contents); // Loading the private key into memory
    openssl_sign($message_data, $signature, $private_key_pem, OPENSSL_ALGO_SHA512); // Compute signature using SHA512
    openssl_free_key($private_key_pem); // Free the private key from memory for additional security

    $pubkey = fopen($public_key_path, "r"); // Open public key file
    if ((!isset($pubkey)) || empty($pubkey)) {
      error_page(tra("Unable to access the required public key. Contact the project administrator to resolve this issue."));
    }
    $pubkey_contents = fread($pubkey, 8192); // Read contents to var
    fclose($pubkey); // Close pub key file

    $base64_sig = base64_encode($signature); // Base64 encode the generated signature to enable safe output to text file.
    $decoded_sig = base64_decode($base64_sig); // Decode base64 sig for use in sig_verification
    $pubkeyid = openssl_pkey_get_public($pubkey_contents); // fetch public key into memory
    $sig_verification = openssl_verify($message_data, $decoded_sig, $pubkeyid, OPENSSL_ALGO_SHA512); // Verify that the generated signature against the original data, using the public key.
    openssl_free_key($pubkeyid); // Free the public key from memory

    // Check if signature was successfully validated
    if ($sig_verification == 1) {
        // The generated signature has been successfully verified using the public key.
        global $master_url; // Define global master_url variable for use in output
        echo "<p>Do not share this information with anyone other than the external system which has requested this proof of account ownership.</p>";
        echo "<textarea rows='13' cols='50' id='openssl_result_textbox'><account_ownership_verification>\n<master_url>$master_url</master_url>\n<msg>$message_data</msg>\n<signature>$base64_sig</signature>\n</account_ownership_verification></textarea>";
        echo "<br/><button onclick='copy_result_textbox()'>Copy text</button>";
        echo '<script type="text/javascript">';
        echo 'function copy_result_textbox() {
              var target_textbox = document.getElementById("openssl_result_textbox");
              target_textbox.select();
              document.execCommand("copy");
              alert("Copied the text: " + copyText.value);
            }';
        echo '</script>';
        page_tail();

    } elseif ($sig_verification == 0) {
        // The generated signature has not been verified. The private/public keys do not match.
        error_page(tra("Signature verification failed. Try again at a later time."));
    } else {
        // Something has gone wrong & an error has occurred.
        error_page(tra("An error occured during the signature verification. Try again at a later time."));
    }
} else {
    // User data input invalid
    error_page(tra("Invalid input. User input must have a length > 0 and < 4096. <form><input type='button' value='Go back!'' onclick='history.back()'></form>"));
}

?>
