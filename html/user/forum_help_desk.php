<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/time.inc');

db_init();

page_head("Questions and answers");

echo "
    Talk live via Skype with a volunteer, in any of several languages.
    Go to
    <a href=http://boinc.berkeley.edu/help.php>BOINC Online Help</a>.
    <p>
    Select a topic or do a
    <a href=forum_search.php>keyword search</a>
    to find what you're looking for.
    <p>
";

start_forum_table(array("Topic", "# Questions", "Last post"));

$categories = getHelpDeskCategories();
while ($category = mysql_fetch_object($categories)) {
    echo "
    <tr class=\"subtitle\">
        <td class=\"category\" colspan=\"4\">", $category->name, "</td>
    </tr>
    ";

    $forums = getForums($category->id);
    while ($forum = mysql_fetch_object($forums)) {
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
?>
