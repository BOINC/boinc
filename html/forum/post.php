<?php

require_once('forum.inc');
require_once('../util.inc');

require_once('subscribe.inc');

if (!empty($_GET['id']) && !empty($_POST['title']) && !empty($_POST['content'])) {
	$_GET['id'] = stripslashes(strip_tags($_GET['id']));

	$user = get_logged_in_user(true, '../');

    if ($_POST['add_signature']=="add_it"){
        $forum_signature = "\n".$user->signature;
    }
    $threadID = createThread($_GET['id'], $user->id, $_POST['title'], $_POST['content'].$forum_signature);

	header('Location: thread.php?id=' . $threadID);
}

if (!empty($_GET['id'])) {
	$forum = getForum($_GET['id']);
	$category = getCategory($forum->category);
} else {
	// TODO: Standard error page
	echo "No forum ID was provided.<br>";
	exit();
}

$logged_in_user = get_logged_in_user(true, '../');

// TODO: Write a function to do this.

if ($category->is_helpdesk) {
	page_head('Help Desk', $logged_in_user);
} else {
	page_head('Forum', $logged_in_user);
}

show_forum_title($forum, NULL, $category->is_helpdesk);

if ($category->is_helpdesk) {
	echo "<p>The <b>Questions and problems</b> area is designed to help you
        get questions answered and problems solved by other users.
        If you have a question or problem:
        <ul>
        <li>
        Read the existing list of questions.
        If your question is already there,
        click on the <b>I also have this question or problem</b> button.
        If answers to the question have been submitted, read them.
        If one of them answers your question, click the
        <b>This answered my question</b> button.
        <li>
        If your question has not already been asked,
        fill out and submit this form.
        </ul>
        The goal is to prevent questions from being asked repeatedly.
    ";
}

echo "<form action=\"post.php?id=", $_GET['id'], "\" method=POST>";

if ($category->is_helpdesk) {
	$cell = "Submit a new question/problem";
} else {
	$cell = "Create a new thread";
}
start_forum_table(array($cell), 2);

echo "<tr><td class=fieldname><b>Title</b>";

if ($category->is_helpdesk) {
	echo "<p>
        Describe your question in a few words.
        A brief, clear summary will help others with the same
        question (or an answer) find it.
        <p></p>
    ";
}

echo "
    </td>
    <td><input type=text name=title size=62></td>
    </tr>
    <tr>
    <td class=fieldname style=\"vertical-align:top\"><b>Message</b>
";

if ($category->is_helpdesk) {
	echo "<p>
        If you are having software problems,
        mention the version number of the software,
        your computer type and operating system.
        </td>
    ";
}
    

echo "
    <td><textarea name=content rows=12 cols=54></textarea></td>
    </tr>
";
row2("", "<input name=add_signature value=add_it checked=true type=checkbox>Add my signature to this post");
row2("", "<input type=submit value=\"OK\">");

end_forum_table();

echo "</form>\n";

page_tail();
?>
