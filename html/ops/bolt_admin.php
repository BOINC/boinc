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
require_once("../inc/util_ops.inc");

function show_course($course) {
    $x = "<b>$course->name</b>
            <br>Description: $course->description
            <br>Created: ".date_str($course->create_time)."
    ";
    $y = "<a href=bolt_map.php?course_id=$course->id>Map</a>
        <br><a href=bolt_compare.php?course_id=$course->id>Experiments</a>
        <br>
    ";
    row2_init($x, $y);
    if ($course->hidden) {
        show_button("bolt_admin.php?action=unhide&course_id=$course->id", "Unhide", "Unhide this course");
    } else {
        show_button("bolt_admin.php?action=hide&course_id=$course->id", "Hide", "Hide this course");
    }
    show_button("bolt_admin.php?action=clear_confirm&course_id=$course->id", "Clear data", "Clear student data for this course");
    echo "</td></tr>";
}

function show_courses() {
    $courses = BoltCourse::enum();
    start_table();
    table_header("Course", "Tools");
    foreach ($courses as $course) {
        show_course($course);
    }
    end_table();
}

function add_course_form() {
    echo "
        <form action=bolt_admin.php method=get>
        <input type=hidden name=action value=add_course>
    ";
    start_table();
    row1("Add course");
    row2("Course name<span class=note><br>Visible to users</span>", "<input name=course_name>");
    row2("Internal name<span class=note><br>Not visible to users; used as a directory name, so no spaces or special chars</span>", "<input name=short_name>");
    row2("Description<span class=note><br>Visible to users</span>", "<textarea name=description cols=60></textarea>");
    row2("", "<input type=submit name=submit value=\"Add course\">");
    end_table();
    echo "</form>";
}

function user_settings() {
    global $user;
    $flags = $user->bolt->flags;
    echo "<form action=bolt_admin.php method=get>
        <input type=hidden name=action value=update_user>
    ";
    start_table();
    row1("User settings");
    $x = ($flags&BOLT_FLAGS_SHOW_ALL)?"checked":"";
    row2("Show hidden courses?", "<input type=checkbox name=show_all $x>");
    $x = ($flags&BOLT_FLAGS_DEBUG)?"checked":"";
    row2("Show debugging output?", "<input type=checkbox name=debug $x>");
    row2("", "<input type=submit name=submit value=\"Update settings\">");
    end_table();
    echo "</form>";
}

function show_all() {
    admin_page_head("Bolt course administration");
    show_courses();
    echo "<p>
        <a href=bolt_admin.php?action=add_course_form>Add course</a>
        <p>
        <a href=bolt_admin.php?action=update_user_form>User settings</a>
    ";
    admin_page_tail();
}

function clear_confirm() {
    global $course_id;

    admin_page_head("Bolt course administration");
    echo "This will clear all student data for this course.
        This is irrevocable.
        Are you sure you want to do this?
        <p>
        <a href=bolt_admin.php?action=clear&course_id=$course_id>Yes</a>
    ";
    admin_page_tail();
}

function clear() {
    global $course_id;

    admin_page_head("Deleting course data");
    BoltEnrollment::delete_aux("course_id = $course_id");
    BoltView::delete_aux("course_id = $course_id");
    BoltResult::delete_aux("course_id = $course_id");
    BoltXsetResult::delete_aux("course_id = $course_id");
    BoltSelectFinished::delete_aux("course_id = $course_id");
    BoltRefreshRec::delete_aux("course_id = $course_id");
    BoltQuestion::delete_aux("course_id = $course_id");
    
    echo "
        Course data deleted.
    ";
    admin_page_tail();
}

$user = get_logged_in_user();

$db = BoltDb::get();
if (!$db) error_page("Can't connect to database server");

if (!$db->table_exists('bolt_course')) {
    page_head("Create Bolt database");
    $db_name = $db->db_name;
    echo "
        The database tables for Bolt don't seem to exist.
        To create them, go to ~/boinc/db and type
        <pre>
mysql $db_name < bolt_schema.sql
</pre>
    Then <a href=bolt_admin.php>reload this page</a>.
    ";
    page_tail();
    exit();
}

BoltUser::lookup($user);
$course_id = get_int('course_id', true);
if ($course_id) $course = BoltCourse::lookup_id($course_id);

$action = get_str('action', true);
switch ($action) {
case 'add_course_form':
    admin_page_head("Add course");
    add_course_form();
    admin_page_tail();
    break;
case 'add_course':
    $short_name = BoltDb::escape_string(get_str('short_name'));
    $name = BoltDb::escape_string(get_str('course_name'));
    $description = BoltDb::escape_string(get_str('description'));
    $now = time();
    BoltCourse::insert("(create_time, short_name, name, description) values ($now, '$short_name', '$name', '$description')");
    Header('Location: bolt_admin.php');
    break;
case 'update_user_form':
    admin_page_head("Bolt user settings");
    user_settings();
    admin_page_tail();
    break;
case 'update_user':
    $flags = 0;
    if (get_str('show_all', true)) $flags |= BOLT_FLAGS_SHOW_ALL;
    if (get_str('debug', true)) $flags |= BOLT_FLAGS_DEBUG;
    $user->bolt->update("flags=$flags");
    $user->bolt->flags = $flags;
    Header('Location: bolt_admin.php');
    break;
case 'hide':
    if (!$course) error_page("no such course");
    $course->update("hidden=1");
    Header('Location: bolt_admin.php');
    break;
case 'unhide':
    if (!$course) error_page("no such course");
    $course->update("hidden=0");
    Header('Location: bolt_admin.php');
    break;
case 'clear_confirm':
    clear_confirm();
    break;
case 'clear':
    clear();
    break;
case '':
    show_all();
    break;
default:
    error_page("unknown action $action");
}


?>
