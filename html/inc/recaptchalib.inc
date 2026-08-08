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

function boinc_recaptcha_get_head_extra() {
    if (recaptcha_public_key()) {
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

// returns true if the captcha was correct
// see https://developers.google.com/recaptcha/docs/verify
//
function boinc_recaptcha_isValidated($privatekey) {
    $url = sprintf('%s?secret=%s&response=%s&remoteip=%s',
        "https://www.google.com/recaptcha/api/siteverify",
        $privatekey,
        htmlspecialchars($_POST['g-recaptcha-response']),
        filter_input(INPUT_SERVER, 'REMOTE_ADDR', FILTER_SANITIZE_URL)
    );
    $response_json = file_get_contents($url);
    $response = json_decode($response_json);
    return (!empty($response->success));
}

?>
