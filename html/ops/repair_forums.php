<?php
require_once("../inc/forum_db.inc");
set_time_limit(0);

// delete threads and posts for non-existent forums

function cleanup_thread($thread) {
    $posts = BoincPost::enum("thread=$thread->id");
    foreach ($posts as $post) {
        $post->delete();
    }
    $thread->delete();
}

function cleanup_orphan_threads() {
    $threads = BoincThread::enum("");
    foreach($threads as $thread) {
        $forum = BoincForum::lookup_id($thread->forum);
        if (!$forum) {
            cleanup_thread($thread);
        }
    }
}

cleanup_orphan_threads();

?>
