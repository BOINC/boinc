<?php
require_once('../include.php');
require_once('forum.inc');
require_once('../util.inc');

if (!empty($_GET['thread']) && !empty($_POST['content'])) {
        $_GET['thread'] = stripslashes(strip_tags($_GET['thread']));
        
        if (!empty($_GET['post'])) {
            $parent_post = $_GET['post'];
        } else {
            $parent_post = NULL;
        }

	//$user = getUserByAuth($_SESSION['authenticator']);
	$user = get_logged_in_user(true);

	$thread = getThread($_GET['thread']);
	$thread->reply($user->id, $_POST['content'], $parent_post);

	header('Location: thread.php?id='.$thread->id);
}

if (empty($_SESSION['authenticator']))
    get_logged_in_user(true, '../');

        
doHeader('Forum');

if (!empty($_GET['post'])) {
    $postId = $_GET['post'];
} else {
    $postId = -1;
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
                if ($post != -1) {
                    
                }
		$posts = $thread->getPosts();
		while ($post = getNextPost($posts)):
			$user = getUser($post->user);
			?>
			<tr style="vertical-align:top">
				<td>
					<p style="font-weight:bold">
					<?php if ($user->has_profile) { ?>
						<a href="../view_profile.php?userid=<?php echo $post->user ?>"><?php echo $user->name ?></a>
					<?php } else { echo $user->name; } ?>
					</p>
					<p style="font-size:8pt">
						Joined: <?php echo date('M j, Y', $user->create_time) ?>
						<br>Posts: <?php echo $user->posts ?>
					</p>
				</td>
				<td>
                                        <p style="font-size:8pt">Posted: <?php echo date('D M j, Y g:i a', $post->timestamp) ?></p>
                                        <p><?php echo nl2br(stripslashes($post->content)) ?></p>
                                </td>
                                        <?php if ($post->id == $postId) {
						show_message_row($thread, $post);
                                        } ?>
                                        
		<?php endwhile;
		
		if ($postId == -1) {
                    show_message_row($thread, $post);
		} ?>
	</table>
 </p>

<?php
doFooter();
?>

<?php

function show_message_row($thread, $post = NULL) {
    global $logged_in_user;
    
    echo "
    <a name=\"input\"></a>
    <tr class = \"row1\" style=\"vertical-align:top\">
        <td>
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
	<p style=\"font-size:8pt\">Your Message</p>
	<form action='reply.php?thread=", $thread->id;

    if ($post) {
        echo "&post=", $post->id;
    }

    echo "' method=\"post\">
	    <textarea name=\"content\" rows=\"12\" cols=\"54\"></textarea><p>
	    <input type=\"submit\" value=\"Post reply\"></p>
	</form>
	";

    echo "</td></tr>";
}

?>