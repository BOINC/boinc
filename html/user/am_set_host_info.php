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
require_once("../inc/xml.inc");

xml_header();

$db = BoincDb::get();
if (!$db) xml_error($retval);

$auth = BoincDb::escape_string(get_str("account_key"));
$user = BoincUser::lookup("authenticator='$auth'");
if (!$user) {
    xml_error(-136);
}

$hostid = get_int("hostid");

$host = BoincHost::lookup_id($hostid);
if (!$host || $host->userid != $user->id) {
    xml_error(-136);
}

$venue = BoincDb::escape_string(get_str("venue"));

$result = $host->update("venue='$venue'");
if ($result) {
    echo "<am_set_host_info_reply>
    <success/>
</am_set_host_info_reply>
";
} else {
    xml_error(-1, "database error");
}

?>
