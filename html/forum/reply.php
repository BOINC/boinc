<?php
require_once('../include.php');
require_once('forum.inc');
require_once('subscribe.inc');
require_once('../util.inc');

if (!empty($_GET['thread']) && !empty($_POST['content'])) {
        $_GET['thread'] = stripslashes(strip_tags($_GET['thread']));
        
        if (!empty($_GET['post'])) {
            $parent_post = $_GET['post'];
        } else {
            $parent_post = NULL;
        }

	$user = get_logged_in_user(true, '../');

	$thread = getThread($_GET['thread']);
	$thread->reply($user->id, $_POST['content'], $parent_post);
        
        notify_subscribers($thread);
        
	header('Location: thread.php?id='.$thread->id);
}

if (empty($_SESSION['authenticator']))
    get_logged_in_user(true, '../');

        
doHeader('Forum');

if (!empty($_GET['post'])) {
    $post = getPost($_GET['post']);
}

$helpdesk = false;

if (!empty($_GET['helpdesk'])) {
	$helpdesk = true;
}

$thread = getThread($_GET['thread']);
$forum = getForum($thread->forum);

$logged_in_user = get_logged_in_user(true);

?>

<p>
	<span class="title"><?php echo $thread->title ?></span>
	<br><a href="index.php"><?php echo $cfg['sitename'] ?> Forum</a> -> <a href="forum.php?id=<?php echo $forum->id ?>"><?php echo $forum->title ?></a>
</p>

<p style="text-align:center">
	<table class="content" border="0" cellpadding="5" cellspacing="0" width="100%">
		<tr>
			<th style="width: 150px">Author</th>
			<th>Message</th>
		</tr>
<?php

// TODO: Use the same sorting method that the user had in the thread view.
//show_posts($thread, $sort_style, $filter, $show_controls=true, $do_coloring=true, $is_helpdesk=false)

show_posts($thread, 'modified-new',-2, false, false, $helpdesk);
show_message_row($thread, $post);
?>

        </table>
</p>

<?php
doFooter();
?>

<?php

function show_message_row($thread, $post=NULL) {
    global $logged_in_user;
    
    echo "
    <tr class = \"row1\" style=\"vertical-align:top\">
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
    if ($post) echo quote_text($post->content, 80);
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