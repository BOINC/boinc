<?php
/**
 * Using this file you can vote for a thread
 * So far it simply votes for the first post in the thread.
 **/

require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');
db_init();

$threadid = get_int('id');
$thread = new Thread($threadid);
$logged_in_user = re_get_logged_in_user();

$posts = $thread->getPosts(0,true);
header("Location: forum_rate.php?choice=p&post=".$posts[0]->getID());
?>
