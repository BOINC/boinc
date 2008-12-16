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

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");

$max_age_days = 60;     // lock threads older than this

$t = time_str(time());
echo "starting at $t\n";
$t = time() - $max_age_days*86400;
$db = BoincDb::get();
if (!$db) die("can't open DB\n");
$db->do_query("update DBNAME.thread set locked=1 where timestamp<$t and locked=0 and sticky=0");
$n = $db->affected_rows();
$t = time_str(time());
echo "finished at $t; locked $n threads\n";
?>
