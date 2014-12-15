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

// script to delete results with no corresponding workunit.
// In theory these shouldn't exist,
// but (because of crashes or bugs) they sometimes do.
// db_purge doesn't get rid of them; this does

$cli_only = true;
require_once("../inc/util_ops.inc");
require_once("../inc/db.inc");

set_time_limit(0);

// get lists of current results and WUs.
// Do selects in this order so that we'll get all relevant results
//
function get_lists() {
    $config = get_config();
    $db_name = parse_config($config, "<db_name>");
    $db_host = parse_config($config, "<db_host>");
    system("mysql $db_name -h $db_host -e \"select workunitid, id from result \" | tail -n +2 | sort -n > dbc_res.dat");
    system("mysql $db_name -h $db_host -e \"select id from workunit\" | tail -n +2 | sort -n > dbc_wu.dat");
}

// N.B.: on Linux we can use join -v to find results without WU.
// But for some reason this doesn't work on Solaris (*&^@#).
// So we roll our own.
// Note: it's
//
function join_lists() {
    $fwu = fopen('dbc_wu.dat', 'r');
    $fres = fopen('dbc_res.dat', 'r');
    $fout = fopen('dbc_out.dat', 'w');
    $wuid = 0;
    $n = 0;
    while (1) {
        $n++;
        if ($n % 1000 == 0) echo "$n\n";
        $x = fgets($fres);
        if (!$x) break;
        list($reswuid, $resid) = sscanf($x, "%d %d");
        if (feof($fwu)) {
            fputs($fout, "$resid\n");
            continue;
        }
        while ($wuid < $reswuid) {
            $y = fgets($fwu);
            if (!$y) {
                $wuid = 999999999999;
                break;
            }
            sscanf($y, "%d", $wuid);
        }
        if ($wuid == $reswuid) continue;
        if ($wuid > $reswuid) {
            fputs($fout, "$resid\n");
            continue;
        }
    }
}

// It would be better to have db_purge delete the results
// (and write them to XML archive)
// but it's not clear how to do this.
// So just delete them.
//
function delete_results() {
    db_init();
    $f = fopen('dbc_out.dat', 'r');
    while (1) {
        $x = fgets($f);
        if (!$x) break;
        $n = sscanf($x, "%d", $resid);
        if ($n != 1) {
            echo "bad line: $x\n";
            continue;
        }
        $result = BoincResult::lookup_id($resid);
        if (!$result) {
            echo "no result $resultid\n";
            continue;
        }
        $wu = BoincWorkunit::lookup_id($result->workunitid);
        if ($wu) {
            echo "result has WU: $resid\n";
            continue;
        }
        echo "deleting $resid\n";

        // uncomment the following to actually delete

        die("edit script to enable deletion\n");
        //_mysql_query("delete from result where id=$resid");
    }
}

get_lists();
join_lists();
delete_results();

?>
