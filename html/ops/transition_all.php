<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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

// transition all WUs

require_once("../inc/boinc_db.inc");
require_once("../inc/util_ops.inc");

$db = BoincDb::get();
$now = time();
$db->do_query("update DBNAME.workunit set transition_time = $now");

admin_page_head("Transition WUs");
echo "The transition time of all WUs has been set to now.
    Monitor the transitioner log to see what happens.
";
admin_page_tail();
?>
