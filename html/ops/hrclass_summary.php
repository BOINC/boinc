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

// Show how many unsent results are committed to each HR class

// TODO: convert to use new DB interface
// TODO: document in the wiki

include_once( "../inc/db.inc" );
include_once( "../inc/util.inc" );
include_once( "../inc/db_ops.inc" );
include_once( "../inc/util_ops.inc" );
include_once( "../inc/prefs.inc" );

$system_string[ 128 ] = "No OS";
$system_string[ 256 ] = "Linux";
$system_string[ 384 ] = "Windows";
$system_string[ 512 ] = "Darwin";
$system_string[ 640 ] = "FreeBSD";

$cpu_string[ 0 ]  = "Unspecified";
$cpu_string[ 1 ]  = "No cpu";
$cpu_string[ 2 ]  = "Intel";
$cpu_string[ 3 ]  = "AMD";
$cpu_string[ 4 ]  = "Macintosh";
$cpu_string[ 5 ]  = "AMD Athlon";
$cpu_string[ 6 ]  = "AMD Duron";
$cpu_string[ 7 ]  = "AMD Sempron";
$cpu_string[ 8 ]  = "AMD Opteron";
$cpu_string[ 9 ]  = "AMD Athlon 64";
$cpu_string[ 10 ] = "AMD Athlon XP";
$cpu_string[ 11 ] = "Intel Xeon";
$cpu_string[ 12 ] = "Intel Celeron";
$cpu_string[ 13 ] = "Intel Pentium";
$cpu_string[ 14 ] = "Intel Pentium II";
$cpu_string[ 15 ] = "Intel Pentium III";
$cpu_string[ 16 ] = "Intel Pentium 4";
$cpu_string[ 17 ] = "Intel Pentium D";
$cpu_string[ 18 ] = "Intel Pentium M";
$cpu_string[ 19 ] = "AMD Athlon MP";
$cpu_string[ 20 ] = "AMD Turion";
$cpu_string[ 21 ] = "Intel Core2";

$query = "SELECT COUNT(workunit.id) AS count FROM workunit LEFT JOIN result ON workunit.id=result.workunitid WHERE result.server_state=2 AND workunit.hr_class=";

function get_mysql_count( $hr_class ) {
    $result = mysql_query("select count(id) as count from workunit where hr_class=" . $hr_class);
    $count = mysql_fetch_object($result);
    mysql_free_result($result);
    return $count->count;
}

function make_reset_url( $hr_class ) {
    return ("<a href=ops_reset_hrclass.php?hr_class=".$hr_class.">".$hr_class."</a>");
}

db_init();

$timestr = time_str(time(0));
$title = "hr_class summary list at ".$timestr;

admin_page_head( $title );

start_table();

row4( "<b>hr_class</b>", "<b>System</b>", "<b>CPU</b>", "<b># unsent results</b>" );

$unsentresults = get_mysql_count( 0 );
row4( make_reset_url( 0 ), $system_string[ 128 ], $cpu_string[ 0 ], $unsentresults  );

for( $system = 2; $system < 6; ++$system ) {
    for( $cpu = 1; $cpu < 22; ++$cpu ) {
        $hr_class=128*$system+$cpu;

        $unsentresults = get_mysql_count( $hr_class );

        row4( make_reset_url( $hr_class ), $system_string[ $system * 128 ], $cpu_string[ $cpu ], $unsentresults  );
    }
}

end_table();

admin_page_tail();

?>
