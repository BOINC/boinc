<?php

require_once('../include/template.inc');
require_once('forum.inc');
require_once('../util.inc');

define("EXCERPT_LENGTH", "120");

// Number of forum topics per page.
// TODO: Make this a constant.
$n = 50;

if (empty($_GET['id'])) {
	// TODO: Standard error page
	echo "Invalid forum ID.<br>";
	exit();
}

/* sanitize variable */
$_GET['id'] = stripslashes(strip_tags($_GET['id']));
$_GET['sort'] = stripslashes(strip_tags($_GET['sort']));

if (!array_key_exists('start', $_GET) || $_GET['start'] < 0) {
	$_GET['start'] = 0;
}

$forum = getForum($_GET['id']);
$category = getCategory($forum->category);

if ($category->is_helpdesk) {
	doHeader('Help Desk');
	$sort_style = 'help-activity-most';
} else {
	doHeader('Forum');
	($_GET['sort'] != NULL) ? $sort_style = $_GET['sort'] : $sort_style = 'modified-new';
}

echo "
	<form action=\"forum.php\" method=\"get\">
		<input type=\"hidden\" name=\"id\" value=", $forum->id, ">
		<table width=100% cellspacing=0 cellpadding=0>
  			<tr valign=\"bottom\">
    			<td align=\"left\" style=\"border:0px\">
";

show_forum_title($forum, NULL, $category->is_helpdesk);

echo "<p>\n<a href=\"post.php?id=", $_GET['id'], "\">";

if ($category->is_helpdesk) {
	echo "Post a New Question";
} else  {
	echo "Post a New Thread / Question";
}

echo "</a>\n</p>\n</td>";

// Only show the sort combo box for normal forums, never for the help desk.
if (!$category->is_helpdesk) {
	echo "<td align=\"right\" style=\"border:0px\">";
	show_combo_from_array("sort", $forum_sort_styles, $sort_style);
	echo "<input type=\"submit\" value=\"Sort\">\n</td>";
}

echo "</tr>\n</table>\n</form>";

// If there are more than the threshold number of threads on the page, only show the
// first $n and display links to the rest
show_page_nav($forum);

if ($category->is_helpdesk) {
	start_forum_table(array("Question", "Answers"), array(NULL, 50));
} else {
	start_forum_table(array("Titles", "Replies", "Author", "Views", "Last Post"), array(NULL, 50, 150, 50, 170));
}

$threads = getThreads($forum->id, $_GET['start'], $n, $sort_style);

while($thread = mysql_fetch_object($threads)) {
	$user = lookup_user_id($thread->owner);
	$first_post = getFirstPost($thread->id);
	$excerpt = sub_sentence($first_post->content, ' ', EXCERPT_LENGTH, true);
	echo "
			<tr style=\"font-size:8pt; text-align:center\">
				<td class=\"col1\" style=\"font-size:10pt; text-align:left\"><a href=\"thread.php?id=", $thread->id, "\"><b>", stripslashes($thread->title), "</b></a><br>
	";

	if ($category->is_helpdesk) {
		echo "<span style=\"font-size:8pt\">", stripslashes($excerpt), "</span>";
	}

	echo "</td>";
	if ($category->is_helpdesk) {
		echo "<td class=\"col2\">", $thread->replies, "</td>";
	} else {
		echo "
			<td class=\"col2\">", $thread->replies, "</td>
			<td class=\"col3\"><a href=\"../show_user.php?userid=", $thread->owner, "\">", $user->name, "</a></td>
			<td class=\"col2\">", $thread->views, "</td>
			<td class=\"col3\" style=\"text-align:right\">", pretty_time_str($thread->timestamp), "</td>
		";
	}

	echo "</tr>";
}

end_forum_table();

if ($forum->threads > $n) {
	echo $gotoStr;
}

doFooter();


function show_page_nav($forum) {
	global $n;
	
	if ($forum->threads > $n) {
		$totalPages = floor($forum->threads / $n);
		$curPage = floor($_GET['start'] / $n);

		$pages = array(0, 1, 2);
		for ($i = -1 ; $i <= 1 ; $i++)
			if ($curPage + $i > 0 && $curPage + $i < $totalPages - 1)
				array_push($pages, $curPage + $i);
		for ($i = -3 ; $i <= -1 ; $i++)
			if ($totalPages + $i > 0)
				array_push($pages, $totalPages + $i);
		$pages = array_unique($pages);
		natsort($pages);
		$pages = array_values($pages);

		$gotoStr = '<p style="text-align:right">Goto page ';

		if ($curPage == 0)
			$gotoStr .= '<span style="font-size:larger; font-weight:bold">1</span>';
		else
			$gotoStr .= '<a href="forum.php?id='.$_GET['id'].'&start='.(($curPage-1)*$n).'">Previous</a> <a href="forum.php?id='.$_GET['id'].'&start=0">1</a>';

		for ($i = 1 ; $i < count($pages)-1 ; $i++) {
			if ($curPage == $pages[$i]) {
				$gotoStr .= ($i > 0 && $pages[$i-1] == $pages[$i] - 1)?', ':' ... ';
				$gotoStr .= '<span style="font-size:larger; font-weight:bold">'.($pages[$i]+1).'</span>';
			} else {
				$gotoStr .= ($i > 0 && $pages[$i-1] == $pages[$i] - 1)?', ':' ... ';
				$gotoStr .= '<a href="forum.php?id='.$_GET['id'].'&start='.($pages[$i]*$n).'">'.($pages[$i]+1).'</a>';
			}
		}

		if ($curPage == $totalPages-1)
			$gotoStr .= ', <span style="font-size:larger; font-weight:bold">'.$totalPages.'</span>';
		else
			$gotoStr .= ', <a href="forum.php?id='.$_GET['id'].'&start='.(($totalPages-1)*$n).'">'.$totalPages.'</a> <a href="forum.php?id='.$_GET['id'].'&start='.(($curPage+1)*$n).'">Next</a>';

		$gotoStr .= '</p>';

		echo $gotoStr;
	}
}
?>