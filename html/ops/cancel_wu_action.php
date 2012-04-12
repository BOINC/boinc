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

require_once("../inc/db.inc");
require_once("../inc/util_ops.inc");

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
