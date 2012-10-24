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

// Perform DB updates.
// Used by "upgrade".
// Can also be run manually in project/html/ops.

require_once("../inc/util_ops.inc");

$db_revision = 0;
if (file_exists("../../db_revision")) {
    $db_revision = (int) file_get_contents("../../db_revision");
}
require_once("db_update.php");

$updates = array();
foreach($db_updates as $db_update) {
    if ($db_update[0] > $db_revision) {
		$db_revision = $db_update[0];
    }
}

echo "\n$db_version\n";

?>
