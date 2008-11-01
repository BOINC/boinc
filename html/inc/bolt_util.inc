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

// Utililty functions for student pages

function info_incomplete($user) {
    if (!$user->bolt->birth_year) return true;
    if (!$user->bolt->sex) return true;
    return false;
}

function birth_year_select($user) {
    $this_year = date("Y");
    $x = "<select name=birth_year>\n";
    for ($i=$this_year-100; $i<$this_year; $i++) {
        $s = ($i == $user->bolt->birth_year)?"selected":"";
        $x .= "<option value=$i $s>$i</option>\n";
    }
    $s = (!$user->bolt->birth_year)?"selected":"";
        $x .= "<option value=$0 $s>Unspecified</option>\n";
    $x .= "</select>\n";
    return $x;
}

function sex_select($user) {
    $x = "<select name=sex>\n";
    $s = ($user->bolt->sex == 0)?"selected":"";
    $x .= "<option value=0 $s>Unspecified</option>\n";
    $s = ($user->bolt->sex == 1)?"selected":"";
    $x .= "<option value=1 $s>Male</option>\n";
    $s = ($user->bolt->sex == 2)?"selected":"";
    $x .= "<option value=2 $s>Female</option>\n";
    $x .= "</select>\n";
    return $x;
}

function request_info($user, $course) {
    page_head("About you");
    echo "
        You may optionally tell us some facts about yourself.
        This information will help us improve this course,
        and will be kept private.
        <p>
        <form action=bolt_sched.php>
        <input type=hidden name=action value=update_info>
        <input type=hidden name=course_id value=$course->id>
    ";
    start_table();
    row2("Birth year", birth_year_select($user));
    row2("Sex", sex_select($user));
    row2("", "<input type=submit value=OK>");
    end_table();
    echo "</form>\n";
    page_tail();
}

//////////// show refresh schedule //////////////

function show_refresh($r) {
    echo "<tr>
        <td>$r->name</td>
        <td>".time_str($r->due_time)."</td>
        <td>
    ";
    if ($r->last_view_id) {
        echo "
            <a href=bolt_sched.php?course_id=$r->course_id&refresh_id=$r->id&action=start>Restart</a>
            | <a href=bolt_sched.php?course_id=$r->course_id&refresh_id=$r->id&action=resume>Resume</a>
        ";
    } else {
        echo "
            <a href=bolt_sched.php?course_id=$r->course_id&refresh_id=$r->id&action=start>Start</a>
        ";
    }
    echo "
        </td>
        </tr>
    ";
}

function show_refreshes() {
    global $user;
    global $course;

    $refreshes = BoltRefreshRec::enum("user_id=$user->id and course_id=$course->id");
    if (!count($refreshes)) return;
    start_table();
    echo "<tr><th colspan=3>Refresh schedule</th></tr>\n";
    foreach ($refreshes as $r) {
        show_refresh($r);
    }
    end_table();
}

?>
