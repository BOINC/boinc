<?php

require_once('forum.inc');
require_once('../util.inc');

require_once('subscribe.inc');

if (!empty($_GET['thread']) && !empty($_POST['content'])) {
	$_GET['thread'] = stripslashes(strip_tags($_GET['thread']));

  if (!empty($_GET['post'])) {
    $parent_post = $_GET['post'];
  } else {
    $parent_post = NULL;
  }

	$user = get_logged_in_user(true, '../');

	replyToThread($_GET['thread'], $user->id, $_POST['content'], $parent_post);
	notify_subscribers($_GET['thread']);

	header('Location: thread.php?id='.$_GET['thread']);
}

$logged_in_user = get_logged_in_user(true, '../');

if (empty($_GET['thread'])) {
	// TODO: Standard error page.
	echo "No thread ID specified.<br>";
	exit();
}

if (!empty($_GET['post'])) {
    $post = getPost($_GET['post']);
}

$helpdesk = false;

// TODO: Just get this from the category perhaps?

if (!empty($_GET['helpdesk'])) {
	$helpdesk = true;
}

$thread = getThread($_GET['thread']);
$forum = getForum($thread->forum);

// TODO: Write a function for this.
if ($helpdesk) {
	page_head('Help Desk', $logged_in_user);
} else {
	page_head('Forum', $logged_in_user);
}

show_forum_title($forum, $thread, $helpdesk);

start_forum_table(array("Author", "Message"), array(150, NULL));

// TODO: Use the same sorting method that the user had in the thread view.

show_posts($thread, 'modified-new',-2, false, false, $helpdesk);
show_message_row($thread, $post);

end_forum_table();

page_tail();

function show_message_row($thread, $post=NULL) {
    global $logged_in_user;

    echo "
    <tr class=\"message\" style=\"vertical-align:top\">
        <td>
        <a name=\"input\"></a>
        <p style=\"font-weight:bold\">
    ";

    if ($logged_in_user->has_profile) {
        echo "<a href=\"../view_profile.php?userid=", $logged_in_user->id, "\">", $logged_in_user->name, "</a>";
    } else {
        echo $logged_in_user->name;
    }

    echo "
</p>
<p style=\"font-size:8pt\">
Joined: ", date('M j, Y', $logged_in_user->create_time), "
</p>
</td>
<td>
	<p style=\"font-size:8pt\">Your Message";
    if ($post) echo " in response to <a href=#$post->id>Message ID $post->id</a>";
    echo "</p>
	<form action='reply.php?thread=", $thread->id;

    if ($post) {
        echo "&post=", $post->id;
    }

    echo "' method=\"post\">
	    <textarea name=\"content\" rows=\"18\" cols=\"80\">";
    if ($post) echo quote_text(stripslashes($post->content), 80);
    echo "</textarea><p>
	    <input type=\"submit\" value=\"Post reply\"></p>
	</form>
	";

    echo "</td></tr>";
}

function quote_text($text, $cols) {
    $quoteChar = ">";

    $lines = explode("\n", $text);

    $lineChars = strlen($quoteChar);
    $final = $quoteChar;

    for ($i = 0; $i < count($lines); $i++) {
        $words = explode(" ", $lines[$i]);

        for ($j = 0; $j < count($words); $j++) {
            $wordLen = strlen($words[$j]);

            if (($lineChars + $wordLen) >= $cols) {
                $final = $final . "\n" . $quoteChar;
                $lineChars = strlen($quoteChar);
            }
            $final = $final . " " . $words[$j];
            $lineChars += $wordLen + 1;
        }

        $final = $final . "\n" . $quoteChar;
        $lineChars = strlen($quoteChar);
    }

    return $final;
}
?>