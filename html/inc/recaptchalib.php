<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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

// recaptcha utilities

// do not include the loader from somewhere else
require('../inc/recaptcha_loader.php');

function boinc_recaptcha_get_head_extra() {
    global $recaptcha_public_key;
    if ($recaptcha_public_key) {
        return '<script src="https://www.google.com/recaptcha/api.js" async defer></script>
        ';
    }
    return "";
}

function boinc_recaptcha_get_html($publickey) {
    if ($publickey) {
        return '<div class="g-recaptcha" data-sitekey="' . $publickey . '"></div>';
    } else {
        return '';
    }
}

// wrapper for ReCaptcha implementation
// returns true if the captcha was correct or no $privatekey was supplied
// everything else means there was an error verifying the captcha
//
function boinc_recaptcha_isValidated($privatekey) {
    if ($privatekey) {
        // tells ReCaptcha to use fsockopen() instead of get_file_contents()
        $recaptcha = new \ReCaptcha\ReCaptcha($privatekey, new \ReCaptcha\RequestMethod\SocketPost());
        $resp = $recaptcha->verify($_POST['g-recaptcha-response'], $_SERVER['REMOTE_ADDR']);
        return $resp->isSuccess();
    }
    return true;
}

?>
