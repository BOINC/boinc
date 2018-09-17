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

define('CONSENT_TYPE_ENROLL','ENROLL');

function consent_to_a_policy($user, $consent_type_id, $consent_flag, $consent_not_required, $source, $ctime = 0) {
    $mys = BoincDb::escape_string($source);
    if ($ctime==0) {
        $mytime = $user->create_time;
    }
    else {
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
        $consent_result = BoincLatestConsent::lookup("userid={$user->id} AND consent_type_id=$ctid AND consent_flag=1");
        if ($consent_result) {
            return TRUE;
        }
    }
    return FALSE;
}

function consent_after_login($user, $perm=true, $next_url = "") {
    send_cookie('tempuserid', $user->id, false);
    send_cookie('tempperm', $perm, false);
    $save_url = $next_url;
    $next_url = "user_agreetermsofuse.php?next_url=$save_url";
    return $next_url;
}

// Checks to see if a particular consent_type name is in
// available. Returns an array of format: (BOOLEAN, INTEGER). The
// boolean is T/F depending on whether that consent_type exists, and
// if checkenabled=TRUE, if the consent_type is enabled/available for
// use. The integer is the consent_type_id- the id from consent_type
// table. If the boolean is FALSE, the integer returned is -1.
function check_consent_type($name, $checkenabled=TRUE) {
    $ct = BoincConsentType::lookup("shortname = '{$name}'");
    if ($ct and ( !$checkenabled or ($ct->enabled)) ) {
        return array(TRUE, $ct->id);
    }
    return array(FALSE, -1);
}
