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

include_once("../inc/boinc_db.inc");
include_once("../inc/util.inc");

function consent_to_a_policy($user, $consent_name, $consent_flag, $consent_not_required, $source, $ctime = 0) {
    $myn = BoincDb::escape_string($consent_name);
    $mys = BoincDb::escape_string($source);
    if ($ctime==0) {
        $mytime = $user->create_time;
    }
    else {
        $mytime = $ctime;
    }
    return BoincConsent::insert(
        "(id, userid, consent_name, consent_time, consent_flag, consent_not_required, source) " .
        "values(0, $user->id, '$myn', $mytime, $consent_flag, $consent_not_required, '$mys')"
    );

}

function consent_after_login($user, $perm=true, $next_url = "") {
    send_cookie('tempuserid', $user->id, false);
    send_cookie('tempperm', $perm, false);
    $save_url = $next_url;
    $consent_result = BoincConsent::lookup("userid={$user->id} AND consent_name='ENROLL' ORDER BY consent_time DESC LIMIT 1");
    if ($consent_result) {
        if ($consent_result->consent_flag != 1) {
            $next_url = "user_agreetermsofuse.php?next_url=$save_url";
        }
        else{
            send_cookie('auth', $user->authenticator, $perm);
            clear_cookie('tempuserid');
            clear_cookie('tempperm');
        }
    }
    else {
        $next_url = "user_agreetermsofuse.php?next_url=$save_url";
    }
    return $next_url;
}

// Checks to see if a particular consent_type name is in available and
// enabled.
function check_consent_type($name) {
    $ct = BoincConsentType::lookup("shortname = '{$name}'");
    if ($ct and ($ct->enabled)) {
        return TRUE;
    }
    return FALSE;
}
