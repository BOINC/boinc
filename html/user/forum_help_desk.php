<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/time.inc');

page_head(PROJECT.': Questions and problems');

show_forum_title(NULL, NULL, true);

echo "
    <p>
    Do a <a href=forum_text_search_form.php>keyword search</a> of messages.
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
	<tr class=\"row1\" style=\"font-size:8pt; text-align:right\">
		<td class=indent style=\"text-align:left\">
			<span style=\"font-size:10pt; font-weight:bold\"><a href=forum_forum.php?id=$forum->id>$forum->title</a></span>
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
