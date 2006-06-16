<?php

/** 
 * Show the links for possible moderation actions related to a single post 
 **/
function post_moderation_links($post){
    if ($post->isHidden()){
        $x = " - <a href=\"forum_moderate_post_action.php?action=unhide&id=".$post->getID()."\">[undelete post]</a> - ";
    } else {
        $x = " - <a href=\"forum_moderate_post.php?action=hide&id=".$post->getID()."\">[delete post]</a> - ";
    }
    $x.= "<a href=\"forum_moderate_post.php?action=move&id=".$post->getID()."\">[move post]</a>";
    return $x;
}

/**
 * Show the links for possible moderation actions related to an entire thread
 **/
function show_thread_moderation_links($thread){
    echo "<a href=\"forum_moderate_thread.php?action=hide&id=".$thread->getID()."\">Hide thread</a><br>";
    if($thread->sticky) {
        echo "<a href=\"forum_moderate_thread_action.php?action=desticky&id=".$thread->getID()."\">De-sticky thread</a><br>"; 
    } else { 
        echo "<a href=\"forum_moderate_thread_action.php?action=sticky&id=".$thread->getID()."\">Make thread sticky</a><br>"; 
    }
}
?>