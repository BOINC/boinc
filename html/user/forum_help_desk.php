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

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/time.inc');

$user = get_logged_in_user(false);

page_head("Questions and answers");

echo "<p>
    Talk live via Skype with a volunteer, in any of several languages.
    Go to
    <a href=\"http://boinc.berkeley.edu/help.php\">BOINC Online Help</a>.</p>
";

show_forum_header($user);

$categories = BoincCategory::enum("is_helpdesk=1 order by orderID");
$first = true;
foreach ($categories as $category) {
    if ($first) {
        $first = false;
        show_forum_title($category, null, null);
        echo "<p>";
        show_mark_as_read_button($user);
        start_forum_table(array("Topic", "# Questions", "Last post"),"id=\"catview\" cellspacing=0");
    }
    if (strlen($category->name)) {
        echo "
            <tr class=\"subtitle\">
            <td class=\"category\" colspan=\"4\">", $category->name, "</td>
            </tr>
        ";
    }

    $forums = BoincForum::enum("parent_type=0 and category=$category->id order by orderID");
    foreach ($forums as $forum) {
        echo "
        <tr class=\"row1\">
        <td>
            <b><a href=\"forum_forum.php?id=$forum->id\">$forum->title</a></b>
            <br><span class=\"smalltext\">", $forum->description, "</span>
        </td>
        <td class=\"numbers\">", $forum->threads, "</td>
        <td class=\"lastpost\">", time_diff_str($forum->timestamp, time()), "</td>
    </tr>
        ";
    }
}

echo "
    </table>
</p>
";

page_tail();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
