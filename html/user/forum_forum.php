<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/time.inc');
require_once('../inc/forum_show.inc');

db_init();

$id = get_int("id");
$sort_style = get_str("sort", true);
$start = get_int("start", true);
if (!$start) $start = 0;

$forum = getForum($id);
$category = getCategory($forum->category);
$logged_in_user = get_logged_in_user(false);
$logged_in_user = getForumPreferences($logged_in_user);

if ($category->is_helpdesk) {
    if (!$sort_style) {
        $sort_style = getSortStyle($logged_in_user,"faq");
    } else {
        setSortStyle($logged_in_user,"faq",$sort_style);
    }
    if (!$sort_style) $sort_style = 'activity';
    page_head('Help Desk');
} else {
    if (!$sort_style) {
        $sort_style = getSortStyle($logged_in_user,"forum");
    } else {
        setSortStyle($logged_in_user, "forum",$sort_style);
    }
    if (!$sort_style) $sort_style = 'modified-new';
    page_head('Message boards : '.$forum->title);
    echo "<link href=\"forum_index.php\" rel=\"up\" title=\"Forum Index\">";
}

echo "
    <form action=forum_forum.php method=get>
    <input type=hidden name=id value=", $forum->id, ">
    <table width=100% cellspacing=0 cellpadding=0>
    <tr valign=bottom>
    <td align=left style=\"border:0px\">
";

show_forum_title($forum, NULL, $category->is_helpdesk);

echo "<p>\n<a href=forum_post.php?id=$id>";

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
show_forum($category, $forum, $start, $sort_style, $logged_in_user);

page_tail();

?>
