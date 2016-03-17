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

$cancel = post_int('cancel', true);
$WU = post_arr('WU', true);
$back = post_str('back', true);
$clause = post_str('clause', true);
$limit = post_int('limit', true);
if (!$limit || $limit == 0) {
    $limit = 20;
}

admin_page_head("Cancel Workunits");

// check for WUs to cancel
//
$WUs = "";
if ($cancel && ($cancel == 1)) {
    if ($WU) {
        foreach ($WU as $key => $value) {
            if($WUs != "") {
                $WUs = $WUs . ",";
            }
            $WUs = $WUs . $value;
        }
    }
}

// cancel WUs (if not in rops)
//
if($WUs != "") {
    echo "<!--- WUs to cancel: $WUs --->\n";
//    if (!in_rops()) {
        cancel_wus_where("id IN (" . $WUs . ")");
//    }
}

if ($back) {
    if ($back == "errorwus") {
        $args = "?refresh_cache=1";
        if (array_key_exists('hide_canceled',$_REQUEST)) {
            $args .= "&hide_canceled=on";
        }
        echo "<p><a href=\"errorwus.php$args\">Return to All-error Workunits page</a></p>";
    } else if ($back == "cancelwus") {
        if ($clause) {
            $clause=urlencode($clause);
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
?>
