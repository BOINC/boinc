<?php
require_once('../include.php');
require_once('forum.inc');
require_once('../util.inc');
require_once('subscribe.inc');

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
$forum = getForum($_GET['id']);

if ($forum->is_helpdesk) {
	doHeader('Help Desk');
} else {
	doHeader('Forum');	
}


?>

<p>
	<span class="title"><?php echo $forum->title ?></span>
	<br><a href="index.php"><?php echo $cfg['sitename'] ?> Forum</a>
</p>
<?php if ($forum->is_helpdesk) { ?>
<p>
The <?php echo PROJECT ?> Help Desk is designed to help users find answers to questions they might have about our project, software, science, etc.<p>To ensure that this is effectively achieved, please make sure to skim the questions that have already been posted before posting your own.  If your question has already been asked by another user, clicking on the "I also have this problem" button in their post will be more effective than re-posting the question.</p><p>If you've been unable to find your question already posted, please fill in the fields below to add it to the Help Desk.</p>
</p>
<?php } ?>
<p style="text-align:center">
	<form action="post.php?id=<?php echo $_GET['id'] ?>" method="post">
		<table class="content" border="0" cellpadding="5" cellspacing="0" width="100%">
			<tr>
				<th colspan="2"><?php if ($forum->is_helpdesk) { echo "Post a New Question"; } else { echo "Post a New Thread / Question"; } ?></th>
			</tr>
			<tr>
				<td><b>Title</b>
				<?php if ($forum->is_helpdesk) { ?>
					<p>Try to describe your question as completely (and concisely) as you can in the space provided.</p>A brief, clear summary will help others with the same question (or an answer to your question) find your post.<p></p>
				<?php } ?>
				</td>
				<td><input type="text" name="title" size="62"></td>
			</tr>
			<tr>
				<td style="vertical-align:top"><b>Message content</b>
				<?php if ($forum->is_helpdesk) { ?>
					<p>Please be as specific as possible: if you are having software problems for example, be sure to mention the version of the software you are running, your computer type, operating system, etc.</p><p>The more detailed your description, the more likely someone will be able to post an answer that solves your problem.</p></td>
				<?php } ?>
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