<?php

require_once('../include/template.inc');
require_once('forum.inc');
require_once('../util.inc');

doHeader('Help Desk', 'forum.css');

show_forum_title(NULL, NULL, true);

echo "<p style=\"text-align:center\">";

start_forum_table(array("Help Desk", "Questions", "Last Answer Posted"), array(NULL, 60, 160));

$categories = getHelpDeskCategories();
while ($category = getNextCategory($categories)) {
	echo "
	<tr class=\"subtitle\">
		<td colspan=\"4\">", $category->name, "</td>
	</tr>
	";
		
	$forums = getForums($category->id);
	while ($forum = getNextForum($forums)) {
		echo "
	<tr style=\"font-size:8pt; text-align:right\">
		<td style=\"text-align:left\">
			<span style=\"font-size:10pt; font-weight:bold\"><a href=\"forum.php?id=", $forum->id, "\">", $forum->title, "</a></span>
			<br>", $forum->description, "
		</td>
		<td>", $forum->threads, "</td>
		<td>", pretty_time_str($forum->timestamp), "</td>
	</tr>
		";
	}
}

echo "
	</table>
</p>
";

doFooter();
?>