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

// Old PHP code put \' and \" into text fields instead of ' and ".
// Fix this.
//
function remove_backslashes($table, $field) {
    $db = BoincDb::get();
    $query = "update DBNAME.$table set $field=replace(replace($field, '\\\\\\\"', '\\\"'), '\\\\\\'', '\'')";
    $db->do_query($query);
}

remove_backslashes("post", "content");
remove_backslashes("profile", "response1");
remove_backslashes("profile", "response2");
remove_backslashes("thread", "title");

//cleanup_orphan_threads();

?>
