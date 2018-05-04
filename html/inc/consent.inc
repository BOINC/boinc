<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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

function consent_to_termsofuse($user, $consent_id, $consent_flag, $consent_not_required, $source, $ctime = 0) {
    $mys = BoincDb::escape_string($source);
    if ($ctime==0) {
        $mytime = $user->create_time;
    }
    else {
        $mytime = $ctime;
    }
    return BoincConsent::insert(
        "(userid, consent_id, consent_time, consent_flag, consent_not_required, source) " .
        "values($user->id, $consent_id, $mytime, $consent_flag, $consent_not_required, '$mys')"
    );

}