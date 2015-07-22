<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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

// delete zombied host records left over after merging computers

require_once("../inc/boinc_db.inc");
require_once("../inc/util_ops.inc");

$db = BoincDb::get();

if (running_from_web_server()) {
    admin_page_head("Remove Zombie Hosts");
}

$retval = $db->do_query("delete h1 from ".$db->db_name.".host as h1 left outer join ".$db->db_name.".result r1 on r1.hostid=h1.id where h1.userid=0 and r1.id is null");

if ($retval) {
    $n = $db->affected_rows();
    echo "$n zombied host records were removed.\n";
} else {
    echo "database error: ".$db->base_error();
}

if (running_from_web_server()) {
    admin_page_tail();
}
?>
