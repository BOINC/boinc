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

// Show results with pending credit for a user

// DEPRECATED - result.claimed_credit not used anymore

require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/xml.inc");

check_get_args(array("format", "authenticator"));

BoincDb::get(true);

$config = get_config();
if (!parse_bool($config, "show_results")) {
    error_page("This feature is turned off temporarily");
}

$format = get_str("format", true);

if ($format == "xml") {
    xml_header();

    $auth = BoincDb::escape_string(get_str('authenticator'));
    $user = BoincUser::lookup("authenticator='$auth'");
    if (!$user) {
        echo "<error>".xml_error(ERR_DB_NOT_FOUND)."</error>\n";
        exit();
    }
    $sum = 0;
    echo "<pending_credit>\n";
    $results = BoincResult::enum("userid=$user->id AND (validate_state=0 OR validate_state=4) AND claimed_credit > 0");
    foreach($results as $result) {
        echo "<result>\n";
        echo "    <resultid>".$result->id."</resultid>\n";
        echo "    <workunitid>".$result->workunitid."</workunitid>\n";
        echo "    <hostid>".$result->hostid."</hostid>\n";
        echo "    <claimed_credit>".$result->claimed_credit."</claimed_credit>\n";
        echo "    <received_time>".$result->received_time."</received_time>\n";
        echo "</result>\n";
        $sum += $result->claimed_credit;
    }
    echo "<total_claimed_credit>".$sum."</total_claimed_credit>\n";
    echo "</pending_credit>\n";
} else {
    $user = get_logged_in_user();
    $sum = 0;

    page_head(tra("Pending credit"));
    start_table();
    echo "<tr><th>".tra("Result ID")."</th><th>".tra("Workunit ID")."</th><th>".tra("Host ID")."</th><th>".tra("Claimed credit")."</th></tr>\n";
    $results = BoincResult::enum("userid=$user->id AND (validate_state=0 OR validate_state=4) AND claimed_credit > 0");
    foreach($results as $result) {
        echo "<tr>\n";
        echo "<td><a href=\"result.php?resultid=$result->id\">$result->id</a></td>\n";
        echo "<td><a href=\"workunit.php?wuid=$result->workunitid\">$result->workunitid</a></td>\n";
        echo "<td><a href=\"show_host_detail.php?hostid=$result->hostid\">$result->hostid</a></td>\n";
        echo "<td>".format_credit($result->claimed_credit)."</td>\n";
        echo "</tr>\n";
        $sum += $result->claimed_credit;
    }
    end_table();

    echo tra("Pending credit: %1", format_credit($sum));
    page_tail();
}

?>
