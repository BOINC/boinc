<?php

require_once('forum.inc');
require_once('../util.inc');

page_head('Message boards', NULL, NULL);

show_forum_title(NULL, NULL, false);

echo "<p>Note: For questions or problems about the ".PROJECT."
    client, server, or web site, please visit the
    <a href=\"help_desk.php\">Help Desk / FAQ</a>.</p>
";

start_forum_table(array("Topic", "Threads", "Posts", "Last post"));
show_forums();
end_table();
page_tail();

function show_forums() {
	$categories = getCategories();
	while ($category = mysql_fetch_object($categories)) {
		echo "
			<tr class=subtitle>
				<td class=category colspan=4>",  $category->name, "</td>
			</tr>
		";

		$forums = getForums($category->id);
		while ($forum = mysql_fetch_object($forums)) {
			echo "
				<tr class=row1 style=\"font-size:8pt; text-align:right\">
				<td class=indent style=\"text-align:left\">
					<span style=\"font-size:10pt; font-weight:bold\">
                    <a href=\"forum.php?id=", $forum->id, "\">", $forum->title,
                    "</a></span>
					<br>", $forum->description, "
				</td>
				<td>", $forum->threads, "</td>
				<td>", $forum->posts, "</td>
				<td>", pretty_time_str($forum->timestamp), "</td>
			</tr>
			";
		}
	}
}
?>
