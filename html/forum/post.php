<?php

require_once('../include/template.inc');
require_once('forum.inc');
require_once('../util.inc');

require_once('subscribe.inc');

if (!empty($_GET['id']) && !empty($_POST['title']) && !empty($_POST['content'])) {
	$_GET['id'] = stripslashes(strip_tags($_GET['id']));

	$user = get_logged_in_user(true);

	$thread = new Thread();
	$thread->forum = $_GET['id'];
	$thread->owner = $user->id;
	$thread->title = $_POST['title'];

	$thread->post($_POST['content']);
        
	header('Location: thread.php?id='.$thread->id);
}

if (!empty($_GET['id'])) {
	$forum = getForum($_GET['id']);
	$category = getCategory($forum->category);
} else {
	// TODO: Standard error page
	echo "No forum id was provided.<br>";
	exit();
}

get_logged_in_user(true, '../');

if ($category->is_helpdesk) {
	doHeader('Help Desk');
} else {
	doHeader('Forum');	
}

show_forum_title($forum, NULL, $category->is_helpdesk);

if ($category->is_helpdesk) {
	echo "<p>The ", PROJECT, " Help Desk is designed to help users find answers to questions they might have about our project, software, science, etc.<p>To ensure that this is effectively achieved, please make sure to skim the questions that have already been posted before posting your own.  If your question has already been asked by another user, clicking on the \"I also have this problem\" button in their post will be more effective than re-posting the question.</p><p>If you've been unable to find your question already posted, please fill in the fields below to add it to the Help Desk.</p></p>";
}

echo "<form action=\"post.php?id=", $_GET['id'], "\" method=\"POST\">";

if ($category->is_helpdesk) {
	$cell = "Post a New Question";	
} else {
	$cell = "Post a New Thread / Question";
}
start_forum_table(array($cell), array(NULL), 2);

echo "<tr><td><b>Title</b>";

if ($category->is_helpdesk) {
	echo "<p>Try to describe your question as completely (and concisely) as you can in the space provided.</p>A brief, clear summary will help others with the same question (or an answer to your question) find your post.<p></p>";
}

echo "
				</td>
				<td><input type=\"text\" name=\"title\" size=62></td>
			</tr>
			<tr>
				<td style=\"vertical-align:top\"><b>Message content</b>
";

if ($category->is_helpdesk) {
	echo "<p>Please be as specific as possible: if you are having software problems for example, be sure to mention the version of the software you are running, your computer type, operating system, etc.</p><p>The more detailed your description, the more likely someone will be able to post an answer that solves your problem.</p></td>";
}

echo "
				<td><textarea name=\"content\" rows=\"12\" cols=\"54\"></textarea></td>
			</tr>
			<tr>
				<td colspan=2 style=\"text-align:center\">
					<input type=\"submit\" value=\"Post message\">
				</td>
			</tr>
";

end_forum_table();

echo "</form>\n";

doFooter();
?>