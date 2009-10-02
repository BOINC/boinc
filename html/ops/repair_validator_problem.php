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

// repairs a problem that was fixed in 2004.
// You should never have to run this script

set_time_limit(0);
$cli_only = true;
require_once("../inc/util_ops.inc");

// activate/deactivate script
if (1) {
  echo "
This script needs to be activated before it can be run.
Once you understand what the script does you can change the 
if (1) to if (0) at the top of the file to activate it.
Be sure to deactivate the script after using it to make sure
it is not accidentally run. 
";
  exit;
}

db_init();

$result = mysql_query("select * from workunit where canonical_resultid=0");
while ($wu = mysql_fetch_object($result)) {
    $r2 = mysql_query("select count(*) from result where workunitid=$wu->id and outcome=1 limit 1000");
    $x = mysql_fetch_array($r2);
    mysql_free_result($r2);
    $nsuccess = $x[0];

    $r2 = mysql_query("select count(*) from result where workunitid=$wu->id and server_state=2");
    $x = mysql_fetch_array($r2);
    mysql_free_result($r2);
    $nunsent = $x[0];

    if ($nsuccess>=3 and $nunsent==0) {
        echo "WU $wu->id has $nsuccess success, $nunsent unsent \n";
        mysql_query("update workunit set need_validate=1 where id=$wu->id");
    }
}

?>
