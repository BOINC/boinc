<?php

require_once('forum.inc');
require_once('../util.inc');

page_head('Forum');

show_forum_title(NULL, NULL, true);

echo "<p style=\"text-align:center\">";

start_forum_table(array("Help Desk", "Questions", "Last Answer Posted"), array(NULL, 60, 160));

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
	<tr class=\"row1\" style=\"font-size:8pt; text-align:right\">
		<td class=\"indent\" style=\"text-align:left\">
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

page_tail();
?>