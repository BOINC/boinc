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
        $func = $db_update[1];
        echo "need update $func\n";
        $updates[] = $db_update;
    }
}

if (empty($updates)) {
    echo "No updates needed\n";
    exit;
}

echo "Do you want to apply these updates? (y/n) ";
$x = trim(fgets(STDIN));
if ($x != 'y') {
    echo "OK - see db_update.php to do manual updates,
or run this script again later.
";
    exit;
}

if ($argc > 1) {
    echo "\nWarning: you are upgrading only web or server code,\nbut these updates may affect the other code as well.\nWe recommend that you run 'upgrade' again to upgrade both parts of the code.\n\n";
}

db_init_cli();

foreach($updates as $update) {
    list($rev, $func) = $update;
    echo "performing update $func\n";
    call_user_func($func);
    file_put_contents("../../db_revision", $rev);
}
echo "All done.\n";

?>
