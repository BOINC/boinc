<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2018 University of California
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

// functions dealing with the consent and consent_type tables.

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");

define('CONSENT_TYPE_ENROLL','ENROLL');

function check_termsofuse() {
    return defined('TERMSOFUSE_FILE') and file_exists(TERMSOFUSE_FILE);
}

function consent_to_a_policy(
    $user, $consent_type_id, $consent_flag, $consent_not_required,
    $source, $ctime = 0
) {
    $mys = BoincDb::escape_string($source);
    if ($ctime==0) {
        $mytime = $user->create_time;
    } else {
        $mytime = $ctime;
    }
    return BoincConsent::insert(
        "(userid, consent_type_id, consent_time, consent_flag, consent_not_required, source) " .
        "values($user->id, $consent_type_id, $mytime, $consent_flag, $consent_not_required, '$mys')"
    );

}

// Checks to see if a user has consented to specfic consent_type_id.
function check_user_consent($user, $consent_name) {
    list($checkct, $ctid) = check_consent_type($consent_name);
    if ($checkct) {
        $consent_result = BoincLatestConsent::lookup(
            "userid=$user->id AND consent_type_id=$ctid AND consent_flag=1"
        );
        if ($consent_result) {
            return TRUE;
        }
    }
    return FALSE;
}

// Check if a particular consent_type name is available.
// Returns an array of format: (BOOLEAN, INTEGER).
// The boolean is T/F depending on whether that consent_type exists,
// and if checkenabled=TRUE, if the consent_type is enabled/available for use.
// The integer is the consent_type_id- the id from consent_type table.
// If the boolean is FALSE, the integer returned is -1.
//
function check_consent_type($name, $checkenabled=TRUE) {
    $name = BoincDb::escape_string($name);
    $ct = BoincConsentType::lookup("shortname = '$name'");
    if ($ct and ( !$checkenabled or ($ct->enabled)) ) {
        return array(TRUE, $ct->id);
    }
    return array(FALSE, -1);
}

// When a user uses the Web site to login, this function checks the
// ENROLL consent and intercepts the login,
// presenting the terms of use page Web form before they can continue.
//
function intercept_login($user, $perm, $in_next_url = "") {
    list($checkct, $ctid) = check_consent_type(CONSENT_TYPE_ENROLL);
    $config = get_config();
    if (parse_bool($config, "enable_login_mustagree_termsofuse")
        and $checkct
        and check_termsofuse()
        and (!check_user_consent($user, CONSENT_TYPE_ENROLL))
    ) {
        // sent user to terms-of-use Web form after login
        $mytoken = create_token($user->id, TOKEN_TYPE_LOGIN_INTERCEPT, TOKEN_DURATION_TWO_HOURS);
        send_cookie('logintoken', $mytoken, false);
        send_cookie('tempuserid', $user->id, false);
        send_cookie('tempperm', $perm, false);
        $save_url = $in_next_url;
        return "user_agreetermsofuse.php?next_url=$save_url";
    } else {
        send_cookie('auth', $user->authenticator, $perm);
        return $in_next_url;
    }
}
