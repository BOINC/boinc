<?php

require_once('forum.inc');
require_once('../util.inc');

/* sanitize variable */
if (empty($_GET['id'])) {
	// TODO: Standard error page
	echo "No thread was specified.<br>";
	exit();
}

$_GET['id'] = stripslashes(strip_tags($_GET['id']));

$sort_style = $_GET['sort'];
$filter_min = $_GET['filter'];

if ($filter_min == NULL || $filter_min < -2 || $filter_min > 2) {
    $filter_min = -2;
}

$thread = getThread($_GET['id']);
incThreadViews($thread->id);

$forum = getForum($thread->forum);
$category = getCategory($forum->category);

$logged_in_user = get_logged_in_user(false);

// TODO: Make these more specific.
if ($category->is_helpdesk) {
	page_head('Help Desk', $logged_in_user);
} else {
	page_head('Forum', $logged_in_user);
}

// TODO: Constant for default sort style and filter values.
if ($sort_style == NULL) {
    $sort_style = "date-old";
}

$is_subscribed = false;

if ($logged_in_user) {
    $result = mysql_query("SELECT * FROM subscriptions WHERE userid = " . $logged_in_user->id . " AND threadid = " . $thread->id);
    if ($result) {
        $is_subscribed = (mysql_num_rows($result) > 0);
    }
}

show_forum_title($forum, $thread, $category->is_helpdesk);

echo "
    <form action=thread.php>
	<input type=hidden name=id value=", $thread->id, ">
	<table width=100% cellspacing=0 cellpadding=0>
    <tr>
    <td align=left>
";

$link = "<a href=reply.php?thread=" . $thread->id;
if ($category->is_helpdesk) {
	$link = $link . "&helpdesk=1#input>Answer this question";
} else {
	$link = $link . "#input>Reply to this thread";
}

echo $link, "</a><br>";

if ($is_subscribed) {
	if ($category->is_helpdesk) {
    	echo "You are subscribed to this question.  ";
	} else {
		echo "You are subscribed to this thread.  ";
	}
	echo "<a href=subscribe.php?action=unsubscribe&thread=$thread->id>Click here to unsubscribe</a>.";
} else {
	if ($category->is_helpdesk) {
    	echo "<a href=subscribe.php?action=subscribe&thread=$thread->id>Subscribe to this question</a>";
	} else {
		echo "<a href=subscribe.php?action=subscribe&thread=$thread->id>Subscribe to this thread</a>";
	}
}

echo "</td>";

echo "<td align=right style=\"border:0px\">";
if ($category->is_helpdesk) {
    show_select_from_array("sort", $answer_sort_styles, $sort_styles);
} else {
    echo "Sort/filter ";
    show_select_from_array("sort", $thread_sort_styles, $sort_style);
    show_select_from_array("filter", $thread_filter_styles, $filter_min);
}
echo "<input type=submit value=OK>\n</td>";

echo "</tr>\n</table>\n</form>\n";

if ($category->is_helpdesk) {
	$headings = array("Author", "Question");
} else {
	$headings = array("Author", "Message");
}

start_forum_table($headings);
show_posts($thread, $sort_style, $filter_min, true, true, $category->is_helpdesk);
end_forum_table();

echo "<p>";

$link = "<a href=reply.php?thread=" . $thread->id;
if ($category->is_helpdesk) {
	$link = $link . "&helpdesk=1#input>Answer this question";
} else {
	$link = $link . "#input>Reply to this thread";
}

echo $link, "</a><br>\n</p>";

page_tail();
?>
