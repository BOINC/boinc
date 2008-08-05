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

require_once("../inc/db.inc");
require_once("../inc/xml.inc");

xml_header();

$retval = db_init_xml();
if ($retval) xml_error($retval);

$auth = get_str("account_key");

$user = lookup_user_auth($auth);
if (!$user) {
    xml_error(-136);
}

$name = urlencode($user->name);
$country = urlencode($user->country);
$postal_code = urlencode($user->postal_code);
$url = urlencode($user->url);
$weak_auth = weak_auth($user);

$ret = "<id>$user->id</id>
<name>$name</name>
<country>$country</country>
<weak_auth>$weak_auth</weak_auth>
<postal_code>$postal_code</postal_code>
<global_prefs>
$user->global_prefs
</global_prefs>
<project_prefs>
$user->project_prefs
</project_prefs>
<url>$url</url>
<send_email>$user->send_email</send_email>
<show_hosts>$user->show_hosts</show_hosts>
<teamid>$user->teamid</teamid>
<venue>$user->venue</venue>";

if ($user->teamid) {
    $team = lookup_team($user->teamid);
    if ($team->userid == $user->id) {
        $ret = $ret . "<teamfounder/>\n";
    }
}

echo "<am_get_info_reply>
    <success/>
    $ret
</am_get_info_reply>
";

?>
