<?php

require_once('../include/template.inc');
require_once('forum.inc');
require_once('../util.inc');

doHeader('Forum', 'forum.css');

show_forum_title(NULL, NULL, false);

echo "<p>Note: For questions or problems pertaining to the ", PROJECT, " client, server, or website, please visit the <a href=\"help_desk.php\">Help Desk / FAQ</a>.</p>";

start_forum_table(array("Forum", "Threads", "Posts", "Last Post"), array(NULL, 60, 60, 160));
show_forums();
end_table();
doFooter();


function show_forums() {
	$categories = getCategories();
	while ($category = mysql_fetch_object($categories)) {
		echo "
			<tr class=\"subtitle\">
				<td colspan=\"4\">",  $category->name, "</td>
			</tr>
		";
		
		$forums = getForums($category->id);
		while ($forum = mysql_fetch_object($forums)) {
			echo "
				<tr style=\"font-size:8pt; text-align:right\">
				<td style=\"text-align:left\">
					<span style=\"font-size:10pt; font-weight:bold\"><a href=\"forum.php?id=", $forum->id, "\">", $forum->title, "</a></span>
					<br>", $forum->description, "
				</td>
				<td>", $forum->threads, "</td>
				<td>", $forum->posts, "</td>
				<td>", date('D M j, Y g:i a', $forum->timestamp), "</td>
			</tr>
			";
		}
	}
}	
?>