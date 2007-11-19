<?php

require_once('../inc/util.inc');
require_once('../inc/time.inc');
require_once('../inc/forum.inc');
require_once('../inc/user.inc');

$userid = get_int("userid");
$offset = get_int("offset", true);
if (!$offset) $offset=0;
$hide = true;
$count = 10;

$user = lookup_user_id($userid);
$logged_in_user = get_logged_in_user(false);

if( $logged_in_user = get_logged_in_user(false) ) {
    BoincForumPrefs::lookup($logged_in_user);
    if ($user->id==$logged_in_user->id ||  $logged_in_user->prefs->privilege(0)) {
        $hide = false;
    }
}
page_head("Posts by $user->name");

if($hide) {
    $posts = BoincPost::enum("user=$userid and hidden=0 order by id desc limit $offset,$count");
} else {
    $posts = BoincPost::enum("user=$userid order by id desc limit $offset,$count");
}
$n = 0;
start_table();
foreach ($posts as $post) {
    $thread = BoincThread::lookup_id($post->thread);
    $forum = BoincForum::lookup_id($thread->forum);
    show_post($post, $thread, $forum, $logged_in_user, 0, $n+$offset+1, false);
    $n++;
}
echo "</table>\n";

if ($n == $count) {
    $offset += $count;
    echo "
        <br><br>
        <a href=forum_user_posts.php?userid=$userid&offset=$offset><b>Next $count posts</b></a>
    ";
}

page_tail();
?>
