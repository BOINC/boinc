#!/usr/bin/env php
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

// lock all threads older than N days

$cli_only = true;
require_once("../inc/util_ops.inc");

$max_age_days = 90;     // lock threads older than this
if ($argc > 2) {
    if ($argv[1] == "--ndays") {
        $max_age_days = $argv[2];
    }
}

$t = time_str(time());
echo "starting at $t\n";
$t = time() - $max_age_days*86400;
$db = BoincDb::get();
if (!$db) die("can't open DB\n");
$db->do_query("update DBNAME.thread, DBNAME.forum set DBNAME.thread.locked=1 where DBNAME.thread.forum=DBNAME.forum.id and DBNAME.forum.parent_type=0 and DBNAME.thread.timestamp<$t and DBNAME.thread.locked=0 and DBNAME.thread.sticky=0");
$n = $db->affected_rows();
$t = time_str(time());
echo "finished at $t; locked $n threads\n";
?>
