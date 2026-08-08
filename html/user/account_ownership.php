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
require_once("../inc/countries.inc");
require_once('../inc/recaptchalib.inc');

check_get_args(array("tnow", "ttok"));

$user = get_logged_in_user();
check_tokens($user->authenticator);

function account_ownership_action($user) {
  // POST request - the user has submitted the form.
  page_head(tra("Proof of account ownership results"), null, null, null, boinc_recaptcha_get_head_extra());

  if (recaptcha_private_key()) {
      // Recaptcha is enabled on the BOINC server
      if (!boinc_recaptcha_isValidated(recaptcha_private_key())) {
          // The user failed to solve the recaptcha prompt - redirect them to an error message!
          error_page(
              tra("Your reCAPTCHA response was not correct. Please try again.")
          );
      }
  }

  // Input is passed in from the openssl_sign_form
  $user_data = htmlentities(post_str("user_data", true), ENT_QUOTES, "UTF-8"); // Convert special characters to html equivelant

  if ((strlen($user_data) > 0) && (strlen($user_data) <= 4096)) {
      require_once("../inc/account_ownership.inc");
      // Check that the private key file exists where specified. If not, redirect to error page.
      if (!file_exists($account_ownership_private_key_file_path)) {
          error_page(tra("The proof of account ownership feature is not set up properly. Contact the project administrator to resolve the issue."));
      }

      // Check that the public key file exists where specified. If not, redirect to error page.
      if (!file_exists($account_ownership_public_key_file_path)) {
          error_page(tra("The proof of account ownership feature is not set up properly. Contact the project administrator to resolve the issue."));
      }

      $privkey = fopen($account_ownership_private_key_file_path, "r"); // Opening private key file
      if (!isset($privkey) || empty($privkey)) {
        error_page(tra("The proof of account ownership feature is not set up properly. Contact the project administrator to resolve the issue."));
      }
      $privkey_contents = fread($privkey, 8192); // Reading contents of private key into var
      fclose($privkey); // Closing private key file

      $userid = $user->id; // Retrieving the user's UserId
      $message_data = "$userid $user_data"; // Create the message which will be signed.

      $private_key_pem = openssl_pkey_get_private($privkey_contents); // Loading the private key into memory
      openssl_sign($message_data, $signature, $private_key_pem, OPENSSL_ALGO_SHA512); // Compute signature using SHA512
      openssl_free_key($private_key_pem); // Free the private key from memory for additional security

      $pubkey = fopen($account_ownership_public_key_file_path, "r"); // Open public key file
      if ((!isset($pubkey)) || empty($pubkey)) {
        error_page(tra("The proof of account ownership feature is not set up properly. Contact the project administrator to resolve the issue."));
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
          $url_tokens = url_tokens($user->authenticator);
          // The generated signature has been successfully verified using the public key.
          $master_url = master_url();
          echo "<p>Do not share this information with anyone other than the external system which has requested this proof of account ownership.</p>";
          echo "<textarea rows='13' cols='50' id='result_textbox'><account_ownership_verification>\n<master_url>$master_url</master_url>\n<msg>$message_data</msg>\n<signature>$base64_sig</signature>\n</account_ownership_verification></textarea>";
          echo "<br/><br/><button class='btn btn-success' onclick='copy_result_textbox()'>Copy text</button>";
          echo "<a href='account_ownership.php?$url_tokens'><button class='btn btn-default'>Go back</button></a>";
          echo '<script type="text/javascript">';
          echo 'function copy_result_textbox() {
                var target_textbox = document.getElementById("result_textbox");
                target_textbox.select();
                document.execCommand("copy");
                alert("Copied to clipboard");
              }';
          echo '</script>';
          page_tail();

      } elseif ($sig_verification == 0) {
          // The generated signature has not been verified. The private/public keys do not match.
          error_page(tra("Signature verification failed. Contact the project administrator to resolve the issue."));
      } else {
          // Something has gone wrong & an error has occurred.
          error_page(tra("An error occurred during the signature verification. Contact the project administrator to resolve the issue."));
      }
  } else {
      // User data input invalid
      error_page(tra("Invalid input. User input must have a length > 0 and < 4096. <form><input type='button' value='Go back!'' onclick='history.back()'></form>"));
  }
}

function account_ownership_form($user) {
  // GET request - the user has navigated to the page.
  page_head(tra("Generate proof of account ownership"), null, null, null, boinc_recaptcha_get_head_extra());

  if ($user) { // Verify the user is logged in
      require_once("../inc/account_ownership.inc");

      if (!file_exists($account_ownership_private_key_file_path)) {
          // Check that the private key file exists where specified. If not, redirect to error page.
          error_page(tra("The proof of account ownership feature is not set up properly. Contact the project administrator to resolve the issue."));
      }

      if (!file_exists($account_ownership_public_key_file_path)) {
          // Check that the public key file exists where specified. If not, redirect to error page.
          error_page(tra("The proof of account ownership feature is not set up properly. Contact the project administrator to resolve the issue."));
      }

      echo "<p>This tool is designed to create a proof of account ownership for external systems.</p>";

      if (recaptcha_public_key()) {
          // Recaptcha configured
          echo "<p>Enter a message with length less than 4096 characters into the input textbox below, solve the captcha then click the 'Generate' button.</p>";
      } else {
          // Recaptcha not configured
          echo "<p>Enter a message with length less than 4096 characters into the input textbox below then click the 'Generate' button.</p>";
      }
      echo "<p>A textbox will then appear which contains your proof of account ownership.";
      echo "<form method=post action=account_ownership.php>";

      echo form_tokens($user->authenticator);
      echo "<textarea rows='4' cols='50' name=user_data type=text size=20 placeholder='Enter text'></textarea><br/><br/>";

      if (recaptcha_public_key()) {
          // Trigger recaptcha!
          form_general("", boinc_recaptcha_get_html(recaptcha_public_key()));
      }

      echo "<input class=\"btn btn-success\" type=submit value='".tra("Generate")."'>";
      echo "</form><br/><hr/>";
  } else {
      // The user is not logged in!
      echo "<p>You need to be logged in to use this functionality.</p>";
  }

  page_tail();
}

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    account_ownership_action($user);
} else {
    account_ownership_form($user);
}

?>
