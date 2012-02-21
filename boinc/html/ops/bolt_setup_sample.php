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

$cli_only = true;
require_once("../inc/bolt_db.inc");
require_once("../inc/util_ops.inc");

$short_name = 'test_course';
$name = 'Test course';
$description = 'This course is a demonstration of Bolt';
$doc_file = 'bolt_course_sample.php';
$now = time();

if (BoltCourse::insert("(create_time, short_name, name, description, doc_file) values ($now, '$short_name', '$name', '$description', '$doc_file')")) {
    echo "all done\n";
} else {
    echo "database error\n";
}

?>
