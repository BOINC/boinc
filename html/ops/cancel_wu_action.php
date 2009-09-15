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

// cancel a WU:
// - mark unsent results as OVER, outcome DIDNT_NEED
// - set CANCELLED bit in WU error mask
//

require_once("../inc/util_ops.inc");

function test_mysql_query($msg) {
    echo "mysql_query($msg)<br/>";
    return 1;
}

// for purposes of testing and seeing queries,
// replace the two instances of mysql_query() below with test_mysql_query().
//
function cancel_wu($wuid1, $wuid2) {
    $command1="update result set server_state=5, outcome=5 where server_state=2 and $wuid1<=workunitid and workunitid<=$wuid2";
    $command2="update workunit set error_mask=error_mask|16 where $wuid1<=id and id<=$wuid2";

    if (!mysql_query($command1)) {
        echo "MySQL command $command1 failed:<br/>unable to cancel unsent results.<br/>";
        return 1;
    } else if (!mysql_query($command2)) {
        echo "MySQL command $command2 failed:<br/>unable to cancel workunits.<br/>";
        return 2;
    }

    // trigger the transitioner (it will set file_delete_state)

    $now = time();
    $query="update workunit set transition_time=$now where $wuid1<=id and id<=$wuid2";
    mysql_query($query);

    return 0;
}

require_once("../inc/db.inc");
require_once("../inc/util_ops.inc");

// REMOVE THE FOLLOWING ONLY AFTER PASSWORD-PROTECTING html/ops
//
if (1) {
    echo "
        WARNING! Make sure the html/ops directory is password-protected,
        then edit html/ops/cancel_wu_action.php by hand to remove this message.
    ";
    exit();
}

admin_page_head("Cancel WU");

db_init();

$wuid1 = $_GET['wuid1'];
$wuid2 = $_GET['wuid2'];

if ($wuid1<1 || $wuid2<$wuid1) {
    echo "<h2>Workunit IDs fail to satisfy the conditions:<br/>
        1 <= WU1 ($wuid1) <= WU2 ($wuid2)<br/>
        Unable to process request to cancel workunits.
        </h2>
    ";
    exit();
}

echo "CANCELLING workunits $wuid1 to $wuid2 inclusive....<br/>";

if (cancel_wu($wuid1, $wuid2)) {
    echo "<h2>Failed in";
} else {
    echo "<h2>Success in";
}
echo " cancelling workunits $wuid1 <= WUID <= $wuid2</h2>";

admin_page_tail();
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
