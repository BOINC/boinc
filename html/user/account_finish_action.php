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
require_once("../inc/util.inc");
require_once("../inc/email.inc");

$auth = post_str("auth");
$name = post_str("name");

if (strlen($name)==0) {
    error_page("You must supply a name for your account");
}
if ($new_name != strip_tags($new_name)) {
    error_page("HTML tags not allowed in name");
}

$country = post_str("country");
if (!is_valid_country($country)) {
    error_page( "invalid country");
}
$country = BoincDb::escape_string($country);
$postal_code = BoincDb::escape_string(strip_tags(post_str("postal_code", true)));
$auth = BoincDb::escape_string($auth);

$name = BoincDb::escape_string($name);
$postal_code = BoincDb::escape_string($postal_code);

$user = BoincUser::lookup("authenticator='$auth'");
if (!$user) {
    error_page("no such user");
}
$retval = $user->update("name='$name', country='$country', postal_code='$postal_code'");
if (!$retval) {
    error_page("database error");
}

// team may have already been joined in create_account RPC.
// if so, skip team-finder
//
if ($user->teamid) {
    Header("Location: home.php");
} else {
    Header("Location: team_search.php");
}
send_cookie('auth', $auth, true);
send_cookie('init', "1", true);
?>
