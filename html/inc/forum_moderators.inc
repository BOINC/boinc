<?php

/* Show the links for possible moderation actions related to a single post */
function show_post_moderation_links($post){
    if ($post->hidden){
	echo " - <a href=\"forum_moderate_post_action.php?action=unhide&id=".$post->id."\">[undelete post]</a> - ";
    } else {
        echo " - <a href=\"forum_moderate_post.php?action=hide&id=".$post->id."\">[delete post]</a> - ";
    }
    echo "<a href=\"forum_moderate_post.php?action=move&id=".$post->id."\">[move post]</a>";
}

function show_thread_moderation_links($thread){
    echo "<a href=\"forum_moderate_thread.php?action=hide&id=".$thread->id."\">Hide thread</a><br>";
    if($thread->sticky) { echo "<a href=\"forum_moderate_thread_action.php?action=desticky&id=".$thread->id."\">De-sticky thread</a><br>"; }
    else { echo "<a href=\"forum_moderate_thread_action.php?action=sticky&id=".$thread->id."\">Make thread sticky</a><br>"; }
}
?>