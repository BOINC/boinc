<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/subscribe.inc');

db_init();

$logged_in_user = get_logged_in_user(true);
$logged_in_user = getForumPreferences($logged_in_user);


if (!empty($_GET['thread']) && !empty($_POST['content'])) {
    $_GET['thread'] = stripslashes($_GET['thread']);

    if (!empty($_GET['post'])) {
        $parent_post = $_GET['post'];
    } else {
        $parent_post = NULL;
    }

    if ($_POST['add_signature']=="add_it"){
        $add_signature=true;    // set a flag and concatenate later
    }  else {
        $add_signature=false;
    }

    replyToThread($_GET['thread'], $logged_in_user->id, $_POST['content'], $parent_post, $add_signature);
    notify_subscribers($_GET['thread']);
    header('Location: forum_thread.php?id='.$_GET['thread']);
}


if (empty($_GET['thread'])) {
    // TODO: Standard error page.
    echo "No thread ID specified.<br>";
    exit();
}

if (!empty($_GET['post'])) {
    $post = getPost($_GET['post']);
}

$thread = getThread($_GET['thread']);
$forum = getForum($thread->forum);
$category = getCategory($forum->category);
$helpdesk = $category->is_helpdesk;


// TODO: Write a function for this.
if ($helpdesk) {
    page_head('Questions and problems');
} else {
    page_head('Message boards');
}

show_forum_title($forum, $thread, $helpdesk);

start_forum_table(array("Author", "Message"));

// TODO: Use the same sorting method that the user had in the thread view.

show_posts($thread, 'modified-new',-2, false, false, $helpdesk);
show_message_row($thread, $category, $post);

end_forum_table();

page_tail();

function show_message_row($thread, $category, $post=NULL) {
    global $logged_in_user;

    echo "
    <tr class=\"message\" style=\"vertical-align:top\">
        <td>
        <a name=\"input\"></a>
        <p style=\"font-weight:bold\">
    ";

    echo "
        Write your message here:
        </td>
        <td>
    ";
    if ($post) {
        echo " reply to <a href=#$post->id>Message ID $post->id</a>:";
    }
    if ($category->is_helpdesk) {
        echo "<br><b>
            Please use this form ONLY to answer or
            discuss this particular question or problem.
        ";
    }
    echo "<form action='forum_reply.php?thread=", $thread->id;

    if ($post) {
        echo "&post=", $post->id;
    }

    echo "' method=post><textarea name=\"content\" rows=\"18\" cols=\"80\">";
    if ($post) echo quote_text(stripslashes($post->content), 80);
    if ($logged_in_user->no_signature_by_default==0){$enable_signature="checked=\"true\"";} else {$enable_signature="";}
    echo "</textarea><p>
        <input type=\"submit\" value=\"Post reply\">
        &nbsp;&nbsp;&nbsp;
        <input name=add_signature value=add_it ".$enable_signature." type=checkbox>Add my signature to this reply                                

        </form>
    ";

    echo "</td></tr>\n";
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
