<?php

require_once('forum.inc');
require_once('../util.inc');
require_once('../include/template.inc');

if ($_POST['submit']) {
    
    $post = getPost($_GET['id']);
    $thread = getThread($post->thread);
    
    $post->update($_POST['content']);
        
    header('Location: thread.php?id='.$thread->id);
}

$logged_in_user = get_logged_in_user();

doHeader('Forum');

if (!empty($_GET['id'])) {
	$post = getPost($_GET['id']);
	$thread = getThread($post->thread);
	$forum = getForum($thread->forum);
	$category = getCategory($forum->category);
} else {
	// TODO: Standard error page
	echo "No post was specified.<br>";
	exit();
}

if ($logged_in_user->id != $post->user) {
    // Can't edit other's posts.
    echo "You are not authorized to edit this post.";
    exit();
}

show_forum_title($forum, $thread, $category->is_helpdesk);

echo "<form action=\"edit.php?id=", $post->id, "\" method=\"POST\">";

start_forum_table(array("Edit Your Post"), array(NULL), 2);

echo "
			<tr>
				<td style=\"vertical-align:top\"><b>Message content</b></td>
				<td><textarea name=\"content\" rows=12 cols=80>", $post->content, "</textarea></td>
			</tr>
			<tr>
				<td colspan=2 style=\"text-align:center\">
					<input type=\"submit\" name=\"submit\" value=\"submit\">
				</td>
			</tr>
";

end_forum_table();

echo "</form>";

doFooter();

?>