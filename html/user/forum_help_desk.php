<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/time.inc');

db_init();

page_head("Questions and problems");

echo "
    <p>
    Do a <a href=forum_text_search_form.php>keyword search</a> of messages.
    <p>
";

start_forum_table(array("Topic", "# Questions", "Last post"));

$categories = getHelpDeskCategories();
while ($category = mysql_fetch_object($categories)) {
    echo "
    <tr class=subtitle>
        <td class=category colspan=4>", $category->name, "</td>
    </tr>
    ";

    $forums = getForums($category->id);
    while ($forum = mysql_fetch_object($forums)) {
        echo "
        <tr class=row1>
        <td>
            <b><a href=forum_forum.php?id=$forum->id>$forum->title</a></b>
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
