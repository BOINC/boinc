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

require_once("../inc/bolt_db.inc");
require_once("../inc/util.inc");
require_once("../inc/bolt_util.inc");

page_head("Courses");

$user = get_logged_in_user(false);
if ($user) {
    BoltUser::lookup($user);
}

$courses = BoltCourse::enum();
start_table();
table_header(
    "Course", "Status"
);
foreach ($courses as $course) {
    if ($course->hidden && !($user->bolt->flags&BOLT_FLAG_SHOW_ALL)) {
        continue;
    }
    $e = $user?BoltEnrollment::lookup($user->id, $course->id):null;
    if ($e) {
        $start = date_str($e->create_time);
        $view = BoltView::lookup_id($e->last_view_id);
        $ago = time_diff(time() - $view->start_time);
        $pct = number_format($view->fraction_done*100, 0);
        $status = "Started $start
            <br>Last visit: $ago ago
            <br>$pct% done
        ";
        if ($view->fraction_done < 1) {
            $status .= "<br><a href=bolt_sched.php?course_id=$course->id&action=resume>Resume</a>
            ";
        }
        $status .= "<br><a href=bolt_sched.php?course_id=$course->id&action=start>Restart</a>
            | <a href=bolt_course.php?course_id=$course->id>History</a>
        ";
    } else {
        $status = "
            <a href=bolt_sched.php?course_id=$course->id&action=start>Start</a>
        ";
    }
    row2_init("<b>$course->name</b>
        <br><span class=note>$course->description</span>",
        $status
    );
    show_refreshes();
    echo "</td></tr>\n";
}
end_table();
page_tail();

?>
