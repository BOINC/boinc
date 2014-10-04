<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// Re-validate jobs.
// Use this if you changed your validator to a more permissive policy,
// and you want to rerun it on some jobs
// (e.g. to grant credit to instances previously marked as invalid)

ini_set ("memory_limit", "2G");

require_once("../inc/util_ops.inc");

function show_form() {
    admin_page_head("Revalidate jobs");
    echo "
        This form lets you re-validate jobs.
        Use this if you changed your validator to a more permissive policy,
        and you want to rerun it on some jobs
        (e.g. to grant credit to instances previously marked as invalid).
        <p>
        <form method=get action=revalidate.php>
        <p>
        Enter a SQL 'where' clause indicating which workunits
        you want to revalidate
        (e.g., <b>id >= 1000 and id < 2000</b>).
        <p>
        where <input name=clause size=60>
        <p>
        <input class=\"btn btn-default\" type=submit value=OK>
        </form>
    ";
    admin_page_tail();
}

function revalidate($clause) {
    $nwus = 0;
    $nresults = 0;

    $wus = BoincWorkunit::enum($clause);
    foreach($wus as $wu) {
        $results = BoincResult::enum("workunitid=$wu->id");
        $n = 0;
        foreach ($results as $result) {
            if ($result->server_state != 5) continue;
            if ($result->outcome != 1) continue;
            if ($result->validate_state < 1) continue;
            $result->update("validate_state=0");
            echo "<br>updated result $result->id\n";
            $n++;
            $nresults++;
        }
        if ($n) {
            $wu->update("need_validate=1");
            echo "<br>updated wu $wu->id\n";
            $nwus++;
        }
    }
    echo "Examined ".count($wus)." jobs.\n";
    echo "$nwus jobs and $nresults instances were marked for revalidation";
}

$clause = get_str("clause", true);
if ($clause) {
    admin_page_head("Jobs marked for revalidation");
    revalidate($clause);
    admin_page_tail();
} else {
    show_form();
}

?>
