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

if (DISABLE_FORUMS) error_page("Forums are disabled");

check_get_args(array());

$user = get_logged_in_user(false);

page_head(tra("Questions and answers"));

echo "<p>".
    tra("Talk live via Skype with a volunteer, in any of several languages. Go to %1 BOINC Online Help %2.", "<a href=\"https://boinc.berkeley.edu/help.php\">", "</a>").
    "</p>";

show_forum_header($user);

$categories = BoincCategory::enum("is_helpdesk=1 order by orderID");
$first = true;
foreach ($categories as $category) {
    if ($first) {
        $first = false;
        echo forum_title($category, null, null);
        echo "<p>";
        show_mark_as_read_button($user);
        start_table('table-striped');
        row_heading_array(array(
            tra("Topic"),
            tra("Questions"),
            tra("Last post")
        ));
    }
    if (strlen($category->name)) {
        echo "
            <tr>
            <td colspan=\"4\">", $category->name, "</td>
            </tr>
        ";
    }

    $forums = BoincForum::enum("parent_type=0 and category=$category->id order by orderID");
    foreach ($forums as $forum) {
        echo "
        <tr>
        <td>
            <a href=\"forum_forum.php?id=$forum->id\">$forum->title</a>
            <br><small>", $forum->description, "</small>
        </td>
        <td>", $forum->threads, "</td>
        <td>", time_diff_str($forum->timestamp, time()), "</td>
    </tr>
        ";
    }
}

echo "
    </table>
</p>
";

page_tail();

?>
