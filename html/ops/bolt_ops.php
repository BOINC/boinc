<?php

require_once("../inc/bolt_db.inc");
require_once("../inc/util_ops.inc");

function show_course($course) {
    echo "<tr>
        <td>Name: $course->name<br>Description: $course->description<br>Created: ".date_str($course->create_time)."</td>
        <td>$course->doc_file</td>
        <td>
    ";
    if ($course->hidden) {
        show_button("bolt_ops.php?action=unhide&course_id=$course->id", "Unhide", "Unhide this course");
    } else {
        show_button("bolt_ops.php?action=hide&course_id=$course->id", "Hide", "Hide this course");
    }
}

function show_courses() {
    $courses = BoltCourse::enum();
    start_table();
    row1("Existing courses", 3);
    table_header("Name/description", "Course document", "");
    foreach ($courses as $course) {
        show_course($course);
    }
    end_table();
}

function add_course_form() {
    echo "
        <form action=bolt_ops.php method=get>
    ";
    start_table();
    row1("Add course");
    row2("Name<span class=note><br>Visible to users</span>", "<input name=course_name>");
    row2("Internal name<span class=note><br>Not visible to users; used as a directory name, so no spaces or special chars</span>", "<input name=short_name>");
    row2("Description<span class=note><br>Visible to users</span>", "<textarea name=description cols=60></textarea>");
    row2("Course document", "<input name=doc_file>");
    row2("", "<input type=submit name=submit value=\"Create course\">");
    end_table();
    echo "</form>";
}

function user_settings() {
    global $user;
    $flags = $user->bolt->flags;
    echo "<form action=bolt_ops.php method=get>";
    start_table();
    row1("User settings");
    $x = ($flags&BOLT_FLAGS_SHOW_ALL)?"checked":"";
    row2("Show hidden courses?", "<input type=checkbox name=show_all $x>");
    $x = ($flags&BOLT_FLAGS_DEBUG)?"checked":"";
    row2("Show debugging output?", "<input type=checkbox name=debug $x>");
    row2("", "<input type=submit name=submit value=\"Update user\">");
    end_table();
    echo "</form>";
}

function show_all() {
    admin_page_head("Bolt course administration");
    show_courses();
    echo "<p>";
    add_course_form();
    echo "<p>";
    user_settings();
    admin_page_tail();
}

$user = get_logged_in_user();
BoltUser::lookup($user);

$submit = get_str('submit', true);
if ($submit == 'Create course') {
    $short_name = BoltDb::escape_string(get_str('short_name'));
    $name = BoltDb::escape_string(get_str('course_name'));
    $description = BoltDb::escape_string(get_str('description'));
    $doc_file = get_str('doc_file');
    $now = time();
    BoltCourse::insert("(create_time, short_name, name, description, doc_file) values ($now, '$short_name', '$name', '$description', '$doc_file')");
    Header('Location: bolt_ops.php');
    exit();
} else if ($submit == 'Update user') {
    $flags = 0;
    if (get_str('show_all', true)) $flags |= BOLT_FLAGS_SHOW_ALL;
    if (get_str('debug', true)) $flags |= BOLT_FLAGS_DEBUG;
    $user->bolt->update("flags=$flags");
    $user->bolt->flags = $flags;
    Header('Location: bolt_ops.php');
    exit();
} else {
    $action = get_str('action', true);
    if ($action) {
        $course_id = get_int('course_id');
        $course = BoltCourse::lookup_id($course_id);
        if (!$course) error_page("no such course");
        switch ($action) {
        case 'hide':
            $course->update("hidden=1");
            break;
        case 'unhide':
            $course->update("hidden=0");
            break;
        default:
            error_page("unknown action $action");
        }
        Header('Location: bolt_ops.php');
        exit();
    }
}

show_all();

?>
