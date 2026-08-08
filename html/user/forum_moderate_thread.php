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

require_once('../inc/util.inc');
require_once('../inc/forum.inc');

check_get_args(array("action", "thread", "ttok", "tnow"));

$logged_in_user = get_logged_in_user();
BoincForumPrefs::lookup($logged_in_user);

if (DISABLE_FORUMS && !is_admin($logged_in_user)) {
    error_page("Forums are disabled");
}

if (!get_str('action')) {
    error_page("unknown action");
}
$thread = BoincThread::lookup_id(get_int('thread'));
if (!$thread) error_page('No such thread');
$forum = BoincForum::lookup_id($thread->forum);

if (!is_moderator($logged_in_user, $forum)) {
    error_page(tra("not authorized"));
}

page_head(tra("Moderate thread '%1'", $thread->title));

echo "<form action=forum_moderate_thread_action.php?thread=$thread->id method=POST>\n";
echo form_tokens($logged_in_user->authenticator);
start_table();

$action = get_str('action');
$get_reason = true;
switch ($action) {
case 'hide':
case 'lock':
    echo "<input type=hidden name=action value=$action>";
    row2("",
        tra("Select the reason category, or write a longer description of why you're hiding or locking the thread; then press OK.")
    );
    row2(tra("Category"),
        "<select class=\"form-control\" name=\"category\">
        <option value=\"1\">".tra("Obscene")."</option>
        <option value=\"2\">".tra("Flame/Hate mail")."</option>
        <option value=\"3\">".tra("Commercial spam")."</option>
        <option value=\"4\">".tra("Other")."</option>
        </select>"
    );
    break;
case 'move':
    if ($forum->parent_type != 0) error_page("Nope");
    echo "<input type=hidden name=action value=move>";
    $selectbox = '<select class="form-control" name="forumid">';
    $categories = BoincCategory::enum();
    foreach ($categories as $category) {
        $forums = BoincForum::enum("category=$category->id");
        foreach ($forums as $f) {
            $selectbox .= '<option value="'.$f->id.'">'.$f->title.'</option>';
        }
    }
    $selectbox .= '</option>';

    row2(tra("Current forum"), $forum->title);
    row2(tra("Destination forum"), $selectbox);
    break;
case 'title':
    echo "<input type=hidden name=action value=title>";
    row2(tra("New title:"),
        "<input size=80 name=\"newtitle\" value=\"".htmlspecialchars($thread->title)."\">"
    );
    break;
case 'delete':
    echo "<input type=hidden name=action value=delete>";
    echo "Are you sure you want to delete this thread and all its posts?
        This action cannot be undone.
        <input class=\"btn btn-warning\" type=submit value=Yes>
    ";
    $get_reason = false;
    break;
default:
    error_page("Unknown action");
}

if ($get_reason) {
    row2(tra("Reason")."<br><p class=\"text-muted\">".tra("Mailed if nonempty")."</p>",
        '<textarea rows="10" class="form-control" name="reason"></textarea>'
    );

    row2(
        "",
        "<input class=\"btn btn-default\" type=\"submit\" name=\"submit\" value=\"".tra("OK")."\">"
    );
}

end_table();

echo "</form>";

page_tail();

?>
