<?php
require_once('../include.php');
require_once('../util.inc');
require_once('forum.inc');
doHeader('Forum');

/* sanitize variable */
$_GET['id'] = stripslashes(strip_tags($_GET['id']));

$thread = getThread($_GET['id']);
$thread->incView();

$forum = getForum($thread->forum);

$logged_in_user = get_logged_in_user(false);
    
?>

<p>
	<span class="title"><?php echo $thread->title ?></span>
<br><a href="index.php"><?php echo $cfg['sitename'] ?> Forum</a> -> <a href="forum.php?id=<?php echo $forum->id ?>"><?php echo $forum->title ?></a>
</p>
<p>
	<a href="reply.php?id=<?php echo $_GET['id'] ?>">Reply to Thread / Question</a>
</p>
<p style="text-align:center">
	<table class="content" border="0" cellpadding="5" cellspacing="0" width="100%">
		<tr>
			<th style="width: 150px">Author</th>
			<th>Message</th>
		</tr>
		<?php

		$n = 0;
		$posts = $thread->getPosts();
		while ($post = getNextPost($posts)):
                    $user = getUser($post->user);
                    $can_edit = $logged_in_user && $user->id == $logged_in_user->id;
			?>
			<tr class="row<?php echo (($n++)%2+1) ?>" style="vertical-align:top">
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
					<p style="font-size:8pt">
						Posted: <?php echo date('D M j, Y g:i a', $post->timestamp) ?><?php if ($can_edit) echo "&nbsp;<a href=\"edit.php?id=$post->id\">[Edit this post]</a>" ?>
                                        	<?php if ($post->modified) echo "<br>Last Modified: ", date('D M j, Y g:i a', $post->modified) ?>
                                        </p>
                                        
					<p><?php echo nl2br(stripslashes($post->content)) ?></p>
				</td>
			</tr>
			<?php
		endwhile;

		?>
	</table>
</p>

<?php
doFooter();
?>
