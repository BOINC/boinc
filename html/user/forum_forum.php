<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/time.inc');
require_once('../inc/forum_show.inc');

if (empty($_GET['id'])) {
    // TODO: Standard error page
    echo "Invalid forum ID.<br>";
    exit();
}

$_GET['id'] = stripslashes(strip_tags($_GET['id']));
$_GET['sort'] = stripslashes(strip_tags($_GET['sort']));

if (!array_key_exists('start', $_GET) || $_GET['start'] < 0) {
    $start = 0;
} else {
    $start = $_GET['start'];
}

$forum = getForum($_GET['id']);
$category = getCategory($forum->category);

if ($category->is_helpdesk) {
    $sort_style = $_GET['sort'];
    if (!$sort_style) {
        $sort_style = $_COOKIE['hd_sort_style'];
    } else {
        setcookie('hd_sort_style', $sort_style, time()+3600*24*365);
    }
    if (!$sort_style) $sort_style = 'activity';
    page_head('Help Desk');
} else {
    $sort_style = $_GET['sort'];
    if (!$sort_style) {
        $sort_style = $_COOKIE['forum_sort_style'];
    } else {
        setcookie('forum_sort_style', $sort_style, time()+3600*24*365);
    }
    if (!$sort_style) $sort_style = 'modified-new';
    page_head('Message boards : '.$forum->title);
}

echo "
    <form action=forum.php method=get>
    <input type=hidden name=id value=", $forum->id, ">
    <table width=100% cellspacing=0 cellpadding=0>
    <tr valign=bottom>
    <td align=left style=\"border:0px\">
";

show_forum_title($forum, NULL, $category->is_helpdesk);

echo "<p>\n<a href=\"forum_post.php?id=", $_GET['id'], "\">";

if ($category->is_helpdesk) {
    echo "Submit a question or problem";
} else  {
    echo "Create a new thread";
}

echo "</a>\n</p>\n</td>";

echo "<td align=right>";
if ($category->is_helpdesk) {
    show_select_from_array("sort", $faq_sort_styles, $sort_style);
} else {
    show_select_from_array("sort", $forum_sort_styles, $sort_style);
}
echo "<input type=submit value=OK></td>\n";

echo "</tr>\n</table>\n</form>";

show_forum($category, $forum, $start, $sort_style);

page_tail();


?>
