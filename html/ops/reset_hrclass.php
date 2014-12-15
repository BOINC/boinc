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

// change all WUs committed to a given HR class back to uncommitted
//
// TODO: document - when/why would you want to do this?
// TODO: use new DB interface

include_once( "../inc/db.inc" );
include_once( "../inc/util.inc" );
include_once( "../inc/db_ops.inc" );
include_once( "../inc/util_ops.inc" );
include_once( "../inc/prefs.inc" );

db_init();

if (get_int('hr_class')) {
    $hr_class = get_int('hr_class');
} else {
    $hr_class = 0;
}

$timestr = time_str(time(0));
$title = "hr_class ".$hr_class." reset at ".$timestr;

admin_page_head($title);

if ($hr_class != 0) {
    $result = _mysql_query("UPDATE workunit SET hr_class=0 WHERE hr_class=".$hr_class);
}

echo $title;

admin_page_tail();

?>
