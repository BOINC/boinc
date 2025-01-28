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

// web interface for canceling WUs according to either
// - ID range
// - ID list
// - SQL where clause

// This page shows the form, and a "confirm" page.
// The actual cancellation is done cancel_workunits_action.php

require_once("../inc/util_ops.inc");

admin_page_head("Cancel Jobs");

$limit = get_int('limit', true);
if (!$limit || $limit==0) {
    $limit = 100;
}

$qclause = "";

$minid = get_int('minid', true);
$maxid = get_int('maxid', true);
$list = get_str('list', true);
$uclause = get_str('uclause', true);
$clause = get_str('clause', true);

if ($minid && $maxid) {
    $qclause = "id >=" . $minid . " AND id <=" . $maxid;
} else if ($list) {
    $qclause = "id IN (" . $list . ")";
} else if ($uclause) {
    $qclause = urldecode($uclause);
} else if ($clause) {
    // the following line is BS, but apparently I can't find another way to pass a
    // double quote (") to the query
    $qclause = str_replace('\"', '"', $clause);
}

if ($qclause == "") {

    // copied from old cancel_wu_form.php
    echo "<p>
    This form may be used to cancel unnecessary or unwanted jobs.
    We recommend that you stop the project before doing this.
    Note that the jobs and their corresponding
    results (if any) are NOT removed from the database.
    Instead, they are marked as 'no longer needed'.
    In most cases you should probably only remove jobs whose results
    are all unsent,
    since otherwise a user will not get credit
    for a result that they might return.
    </p>
    <p>
    Please specify jobs by ID range, ID list or clause to be used in a
    'SELECT FROM workunit WHERE' query.
    </p>
    <p>
    You will be given a list of jobs matching your specification
    for confirmation before these are actually canceled.
    </p>
";

    $page = $_SERVER["REQUEST_URI"];
    echo "<form action=\"$page\" method=\"get\" enctype=\"application/x-www-form-urlencoded\">\n";
    echo "<ul><li>\n";
    echo ' Range: ID of first WU to cancel ';
    echo '<input name="minid" type="text" size="10" maxlength="15">';
    echo ' ID of last WU to cancel ';
    echo '<input name="maxid" type="text" size="10" maxlength="15">';
    echo "</li><li>\n";
    echo ' Comma-separated list of IDs ';
    echo '<input name="list" type="text" size="100" maxlength="200">';
    echo "</li><li>\n";
    echo ' WHERE clause (w/o "WHERE") ';
    echo '<input name="clause" type="text" size="100" maxlength="200">';
    echo "</li></ul>\n";
    echo "<p>\n";
    echo ' Limit ';
    echo "<input name=\"limit\" type=\"text\" size=\"3\" maxlength=\"3\" value=\"$limit\">";
    echo "</p>\n";
    echo "<p>\n";
    echo '<input type="submit" value="Cancel jobs">';
    echo "</p>\n";
    echo "</form>\n";

} else { // if ($qclause)

    $query = "SELECT id, name FROM workunit WHERE canonical_resultid = 0 AND error_mask = 0 AND $qclause;";
    $db = BoincDb::get(true);
    $dbresult = $db->do_query($query);

    if (!$dbresult) {
        echo "Error in query '$query'<br>\n";
    } else {

        echo "<form action=\"cancel_workunits_action.php\" method=\"post\">\n";
        echo "<input type=\"hidden\" name=\"back\" value=\"cancelwus\"/>";

        echo "<br><table border=\"1\">\n";
        echo "<tr><th>WU ID</th><th>WU name</th></tr>\n";

        $rescount = 0;
        while ($res = $dbresult->fetch_object()) {
            if ($rescount < $limit) {
                $id = $res->id;
                echo "<tr>\n";

                echo "<td align=\"left\" valign=\"top\">";
                echo "<input type=\"checkbox\" name=\"WU[]\" value=\"$id\" checked>\n";
                echo "<a href=db_action.php?table=workunit&detail=high&id=";
                echo $id;
                echo ">";
                echo $id;
                echo "</a>";
                echo "</td>\n";

                echo "<td align=\"left\" valign=\"top\">";
                echo $res->name;
                echo "</td>\n";

                echo "</tr>\n";
            }
            $rescount++;
        } // while (fetch_object())

        $dbresult->free();

        echo "</table>\n<p>";
        echo $rescount;
        echo " WUs match the query ($query)\n</p>";

        echo "<p>";
        echo "<input type=\"hidden\" name=\"cancel\" value=\"1\"/>";
        echo "<input type=\"hidden\" name=\"limit\" value=\"$limit\"/>";
        $eclause = urlencode($qclause);
        echo "<input type=\"hidden\" name=\"clause\" value=\"$eclause\"/>";
        echo "<input type=\"submit\" value=\"Cancel checked WUs\">";
        echo "</p>\n";
        echo "</form>\n";

    } // if (!$dbresult)

} // if ($qclause)

admin_page_tail();

?>
