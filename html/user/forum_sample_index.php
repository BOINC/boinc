<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/time.inc');

db_init();

page_head('Message boards');

//show_forum_title(NULL, NULL, false);

echo "<p>
    If you have a question or problem, please use the
    <a href=forum_help_desk.php>Questions/problems</a>
    area instead of the Message boards.</p>
    <p>
    Do a <a href=forum_text_search_form.php>keyword search</a>
    of messages.
    <p>
";

function show_category($category) {
    echo "
        <tr class=subtitle>
            <td class=category colspan=4>",  $category->name, "</td>
        </tr>
    ";

    $forums = getForums($category->id);
    while ($forum = mysql_fetch_object($forums)) {
        show_forum_summary($forum);
    }
}

function show_forums() {
    $categories = getCategories();
    while ($category = mysql_fetch_object($categories)) {
        show_category($category);
    }
}

start_forum_table(array("Topic", "Threads", "Posts", "Last post"));
show_forums();
end_table();
page_tail();
?>
