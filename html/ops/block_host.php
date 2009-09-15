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

// limit a given host to 1 job per day

// TODO: document; use new DB interface

include_once( "../inc/db.inc" );
include_once( "../inc/util.inc" );
include_once( "../inc/db_ops.inc" );
include_once( "../inc/util_ops.inc" );
include_once( "../inc/prefs.inc" );

db_init();

if (get_int('hostid')) {
    $hostid = get_int( 'hostid' );
} else {
    error_page("no hostid");
}

$timestr = time_str(time(0));
$title = "host ".$hostid." max_results_day set to 1 at ".$timestr;

admin_page_head($title);

if($hostid > 0) {
    $result = mysql_query("UPDATE host SET max_results_day=1 WHERE id=".$hostid);
}

echo $title;

admin_page_tail();

?>
