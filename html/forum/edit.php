<?php
require_once('../include.php');
require_once('forum.inc');
require_once('../util.inc');

if ($_POST['submit']) {
    
    $post = getPost($_GET['id']);
    $thread = getThread($post->thread);
    
    $post->update($_POST['content']);
        
    header('Location: thread.php?id='.$thread->id);
}

doHeader('Forum');

$logged_in_user = get_logged_in_user();
$post = getPost($_GET['id']);
if ($logged_in_user->id != $post->user) {
    // Can't edit other's posts.
    echo "You are not authorized to edit this post.";
    exit();
}
?>

<p>
	<span class="title">Edit Post</span>
	<br><a href="index.php"><?php echo $cfg['sitename'] ?> Forum</a>
</p>

<p style="text-align:center">
	<form action="edit.php?id=<?php echo $post->id ?>" method="POST">
		<table class="content" border="0" cellpadding="5" cellspacing="0" width="100%">
			<tr>
				<th colspan="2">Edit Your Post</th>
			</tr>
                        <tr>
				<td style="vertical-align:top"><b>Message content</b></td>
				<td><textarea name="content" rows="12" cols="54"><?php echo $post->content ?></textarea></td>
			</tr>
			<tr>
				<td colspan="2" style="text-align:center">
					<input type="submit" name="submit" value="submit">
				</td>
			</tr>
		</table>
	</form>
</p>

<?php doFooter() ?>