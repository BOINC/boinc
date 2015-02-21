<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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

require_once("../inc/util_ops.inc");

admin_page_head("Cancel Workunits");

// check for WUs to cancel
//
$WUs = "";
if (array_key_exists('cancel', $_REQUEST) && ($_REQUEST['cancel'] == 1)) {
    if (is_array($_REQUEST['WU'])) {
        foreach ($_REQUEST['WU'] as $key => $value) {
            if($WUs != "")
                $WUs = $WUs . ",";
            $WUs = $WUs . $value;
        }
    }
}

// cancel WUs (if not in rops)
//
if($WUs != "") {
    echo "<!--- WUs to cancel: $WUs --->\n";
//    if (!in_rops()) {
        db_init();
        cancel_wus_where("id IN (" . $WUs . ")");
//    }
}

if (array_key_exists('back',$_REQUEST)) {
    if ($_REQUEST['back'] == "errorwus") {
        echo "<p><a href=\"errorwus.php\">Return to All-error Workunits page</a></p>";
    } else if ($_REQUEST['back'] == "cancelwus") {
        if (array_key_exists('clause', $_REQUEST)) {
            $limit = 20;
            if (array_key_exists('limit', $_REQUEST))
                $limit=$_REQUEST['limit'];
            $clause=urlencode($_REQUEST['clause']);
            echo "<p><a href=\"cancel_workunits.php?limit=$limit&uclause=$clause\">";
            echo "Cancel next (max $limit) Workunits</a></p>";
        }
        echo "<p><a href=\"cancel_workunits.php\">Return to Cancel Workunits page</a></p>";
    }
}

echo "<p>";
echo "Page last updated ";
echo time_str(time());
echo "</p>\n";

admin_page_tail();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
