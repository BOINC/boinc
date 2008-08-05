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
require_once("../inc/host.inc");

$user = get_logged_in_user();

$hostid = get_int("hostid");
$host = BoincHost::lookup_id($hostid);
if (!$host || $host->userid != $user->id) {
    error_page("We have no record of that computer.");
}

$nresults = host_nresults($host);
if ($nresults == 0) {
    $host->delete();
} else {
    error_page(
        "You can not delete our record of this computer because our
        database still contains work for it.
        You must wait a few days until the work for this computer
        has been deleted from the project database."
    );
}
page_head("Delete record of computer");
echo "
    Record deleted.
    <p><a href=hosts_user.php>Return to list of your computers</a>
";
page_tail();

?>
