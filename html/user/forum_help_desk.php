<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/time.inc');

get_logged_in_user(false);

page_head("Questions and answers");

echo "<p>
    Talk live via Skype with a volunteer, in any of several languages.
    Go to
    <a href=\"http://boinc.berkeley.edu/help.php\">BOINC Online Help</a>.</p>
";

show_forum_title(null, null, null, null, false);
start_forum_table(array("Topic", "# Questions", "Last post"));

$categories = BoincCategory::enum("is_helpdesk=1 order by orderID");
foreach ($categories as $category) {
    echo "
    <tr class=\"subtitle\">
        <td class=\"category\" colspan=\"4\">", $category->name, "</td>
    </tr>
    ";

    $forums = BoincForum::enum("category=$category->id order by orderID");
    foreach ($forums as $forum) {
        echo "
        <tr class=\"row1\">
        <td>
            <b><a href=\"forum_forum.php?id=$forum->id\">$forum->title</a></b>
            <br>", $forum->description, "
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

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
