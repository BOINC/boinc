<?php
require_once('../include.php');
require_once('forum.inc');
require_once('../util.inc');

if (!empty($_GET['id']) && !empty($_POST['title']) && !empty($_POST['content'])) {
	$_GET['id'] = stripslashes(strip_tags($_GET['id']));

	//$user = getUserByAuth($_SESSION['authenticator']);
	$user = get_logged_in_user(true);

	$thread = new Thread();

	$thread->forum = $_GET['id'];
	$thread->owner = $user->id;
	$thread->title = $_POST['title'];

	$thread->post($_POST['content']);

	header('Location: thread.php?id='.$thread->id);
}

if (empty($_SESSION['authenticator'])) {
    get_logged_in_user(true, '../');


}
doHeader('Forum');

$forum = getForum($_GET['id']);
?>

<p>
	<span class="title"><?php echo $forum->title ?></span>
	<br><a href="index.php"><?php echo $cfg['sitename'] ?> Forum</a>
</p>

<p style="text-align:center">
	<form action="post.php?id=<?php echo $_GET['id'] ?>" method="post">
		<table class="content" border="0" cellpadding="5" cellspacing="0" width="100%">
			<tr>
				<th colspan="2">Post a New Thread / Question</th>
			</tr>
			<tr>
				<td style="width:150px"><b>Title</b></td>
				<td><input type="text" name="title" size="62"></td>
			</tr>
			<tr>
				<td style="vertical-align:top"><b>Message content</b></td>
				<td><textarea name="content" rows="12" cols="54"></textarea></td>
			</tr>
			<tr>
				<td colspan="2" style="text-align:center">
					<input type="submit" value="Post message">
				</td>
			</tr>
		</table>
	</form>
</p>

<?php
doFooter();
?>