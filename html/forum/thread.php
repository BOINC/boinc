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

$is_subscribed = false;

if ($logged_in_user) {
    $result = sql_query("SELECT * FROM subscriptions WHERE (userid = " . $logged_in_user->id . ") AND (threadid = " . $thread->id . ")");
    if ($result) {
        $is_subscribed = (sql_num_rows($result) > 0);
    }
}
?>

<p>
<span class="title"><?php echo $thread->title ?></span>
<br><a href="index.php"><?php echo $cfg['sitename'] ?> Forum</a> -> <a href="forum.php?id=<?php echo $forum->id ?>"><?php echo $forum->title ?></a>
</p>
<p>
<a href=reply.php?thread=<?php echo $thread->id ?>#input>Reply to this thread</a><br>
<?php
if ($is_subscribed) {
    echo "You are currently subscribed to this thread.  <a href=subscribe.php?action=unsubscribe&thread=$thread->id>Click here to unsubscribe</a>.";
} else {
    echo "<a href=subscribe.php?action=subscribe&thread=$thread->id>Subscribe to this thread</a>";
}
?>
</p>
<p style="text-align:center">
	<table class="content" border="0" cellpadding="5" cellspacing="0" width="100%">
		<tr>
			<th style="width: 150px">Author</th>
			<th>Message</th>
		</tr>
		<?php show_posts($thread, true, true) ?>
	</table>
</p>
<p>
<a href=reply.php?thread=<?php echo $thread->id ?>#input>Reply to this thread</a><br>
</p>
<?php
doFooter();
?>
