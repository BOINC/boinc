<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');

db_init();

$logged_in_user = get_logged_in_user();
if ($_POST['submit']) {    
    
    if (empty($_GET['id'])) {
        // TODO: Standard error page
        echo "Invalid post ID.<br>";
        exit();
    }
        
    $post = getPost($_GET['id']);
    $thread = getThread($post->thread);

    if ($logged_in_user->id != $post->user) {
        // Can't edit other's posts.
        echo "You are not authorized to edit this post.";
        exit();
    }
    
    updatePost($post->id, $_POST['content']);
    if ($post->parent_post==0 and $thread->owner==$logged_in_user->id){
        updateThread($thread->id, $_POST['title']);
    }

    header('Location: forum_thread.php?id='.$thread->id);
}


page_head('Forum');

if (!empty($_GET['id'])) {
    $post = getPost($_GET['id']);
    $thread = getThread($post->thread);
    $forum = getForum($thread->forum);
    $category = getCategory($forum->category);
} else {
    // TODO: Standard error page
    echo "No post was specified.<br>";
    exit();
}

if ($logged_in_user->id != $post->user) {
    // Can't edit other's posts.
    echo "You are not authorized to edit this post.";
    exit();
}

show_forum_title($forum, $thread, $category->is_helpdesk);

echo "<form action=forum_edit.php?id=$post->id method=POST>\n";

start_table();
row1("Edit your post");
//start_forum_table(array("Edit your post"), array(NULL), 2);
if ($post->parent_post==0 and $thread->owner==$logged_in_user->id) {
	//If this is the first post enable the user to change title
    row2(
	    "Thread title",
	    "<input type=text name=title value=\"".stripslashes($thread->title)."\">"
    );
};

row2(
    "Message content",
    "<textarea name=\"content\" rows=12 cols=80>".stripslashes($post->content)."</textarea>"
);
row2(
    "",
    "<input type=submit name=submit value=OK>"
);

end_table();

echo "</form>";

page_tail();

?>
