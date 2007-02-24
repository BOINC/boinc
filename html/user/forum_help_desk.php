<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/time.inc');

db_init();

page_head("Questions and answers");

echo "<p>
    Talk live via Skype with a volunteer, in any of several languages.
    Go to
    <a href=\"http://boinc.berkeley.edu/help.php\">BOINC Online Help</a>.</p>
    <p>
    <form action=\"forum_search_action.php\" method=\"POST\">
    <input type=\"hidden\" name=\"search_max_time\" value=\"30\">
    <input type=\"hidden\" name=\"search_forum\" value=\"-1\">
    <input type=\"hidden\" name=\"search_sort\" value=\"5\">
    <input type=\"text\" name=\"search_keywords\">
    <input type=\"submit\" value=\"Start forum search\">
    <span class=\"smalltext\">or do <a href=\"forum_search.php\">advanced search</a></span>
    </form>
    </p>
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
