<?php
require_once('../include.php');
require_once('forum.inc');
require_once('../util.inc');

if (!empty($_GET['id']) && !empty($_POST['content'])) {
	$_GET['id'] = stripslashes(strip_tags($_GET['id']));

	//$user = getUserByAuth($_SESSION['authenticator']);
	$user = get_logged_in_user(true);

	$thread = getThread($_GET['id']);
	$thread->reply($user->id, $_POST['content']);

	header('Location: thread.php?id='.$thread->id);
}

if (empty($_SESSION['authenticator']))
	header('Location: ../login_form.php');

doHeader('Forum');

$thread = getThread($_GET['id']);
$forum = getForum($thread->forum);
?>

<p>
	<span class="title"><?php echo $thread->title ?></span>
	<br><a href="index.php"><?php echo $cfg['sitename'] ?> Forum</a> -> <a href="forum.php?id=<?php echo $forum->id ?>"><?php echo $forum->title ?></a>
</p>

<p style="text-align:center">
	<form action="reply.php?id=<?php echo $_GET['id'] ?>" method="post">
		<table class="content" border="0" cellpadding="5" cellspacing="0" width="100%">
			<tr>
				<th colspan="2">Reply to Thread / Question</th>
			</tr>
			<tr>
				<td style="vertical-align:top; width:150px"><b>Message content</b></td>
				<td><textarea name="content" rows="12" cols="54"></textarea></td>
			</tr>
			<tr>
				<td colspan="2" style="text-align:center">
					<input type="submit" value="Post reply">
				</td>
			</tr>
		</table>
	</form>
</p>

<p style="text-align:center">
	<table class="content" border="0" cellpadding="5" cellspacing="0" width="100%">
		<tr>
			<th style="width: 150px">Author</th>
			<th>Message</th>
		</tr>
		<?php

		$posts = $thread->getPosts();
		while ($post = getNextPost($posts)):
			$user = getUser($post->user);
			?>
			<tr style="vertical-align:top">
				<td>
					<p style="font-weight:bold">
						<a href="../view_profile.php?userid=<?php echo $post->user ?>"><?php echo $user->name ?></a>
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
			</tr>
			<?php
		endwhile;

		?>
	</table>
</p>

<?php
doFooter();
?>
