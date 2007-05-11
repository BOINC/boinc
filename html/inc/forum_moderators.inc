<?php

/** 
 * Show the links for possible moderation actions related to a single post 
 **/
function post_moderation_links($config,$logged_in_user,$post,$tokens){
    $moderators_allowed_to_ban = parse_bool($config, "moderators_allowed_to_ban"); 

    if ($post->isHidden()){
        $x = " - <a href=\"forum_moderate_post_action.php?action=unhide&id=".$post->getID()."$tokens\">[undelete post]</a>";
    } else {
        $x = " - <a href=\"forum_moderate_post.php?action=hide&id=".$post->getID()."$tokens\">[delete post]</a> ";
    }
    $x.= " - <a href=\"forum_moderate_post.php?action=move&id=".$post->getID()."$tokens\">[move post]</a>";
    if ($logged_in_user->isSpecialUser(S_ADMIN) || ($logged_in_user->isSpecialUser(S_MODERATOR) && $moderators_allowed_to_ban)) {
      $x .= " - <a href=forum_moderate_post.php?action=banish_user&id=".$post->getID()."&userid=".$post->getOwnerID()."$tokens>[banish author]</a>";
    }
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
