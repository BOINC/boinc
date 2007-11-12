<?php
// Use this file you can vote for a thread.
// It simply votes for the first post in the thread.

require_once('../inc/forum.inc');

$threadid = get_int('id');
$thread = BoincThread::lookup_id($threadid);
$logged_in_user = get_logged_in_user();

$posts = get_thread_posts($threadid, 0,true);
header("Location: forum_rate.php?choice=p&post=".$posts[0]->id);
?>
