<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/time.inc');

page_head('Message boards');

show_forum_title(NULL, NULL, false);

echo "<p>
    If you have a question or problem, please use the
    <a href=help_desk.php>Questions/problems</a>
    area instead of the Message boards.</p>
    <p>
    Do a <a href=text_search_form.php>keyword search</a>
    of messages.
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
            $x = time_diff_str($forum->timestamp, time());
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
				<td>", $x, "</td>
			</tr>
			";
		}
	}
}
?>
