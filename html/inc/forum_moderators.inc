<?php

require_once('../inc/forum_banishment_vote.inc');

// Show the links for possible moderation actions related to a single post 
//
function show_post_moderation_links($config, $logged_in_user, $post, $tokens){
    $moderators_allowed_to_ban = parse_bool($config, "moderators_allowed_to_ban"); 
    $moderators_vote_to_ban = parse_bool($config, "moderators_vote_to_ban");

    if ($post->isHidden()){
        show_button("forum_moderate_post_action.php?action=unhide&id=".$post->getID()."$tokens", "Undelete", "Undelete this post");
    } else {
        show_button("forum_moderate_post.php?action=hide&id=".$post->getID()."$tokens", "Delete", "Delete this post");
    }
    show_button("forum_moderate_post.php?action=move&id=".$post->getID()."$tokens", "Move", "Move post to a different thread");
    if ($logged_in_user->isSpecialUser(S_ADMIN) || ($logged_in_user->isSpecialUser(S_MODERATOR) && $moderators_allowed_to_ban)) {
        show_button("forum_moderate_post.php?action=banish_user&id=".$post->getID()."&userid=".$post->getOwnerID()."$tokens", "Banish author", "Banish author");
    }
    if ($logged_in_user->isSpecialUser(S_MODERATOR) && $moderators_vote_to_ban) {
        if (vote_is_in_progress($post->getOwnerID())) {
           show_button("forum_banishment_vote.php?action=yes&userid=".$post->getOwnerID(), "Vote to banish author", "Vote to banish author");
           show_button("forum_banishment_vote.php?action=no&userid=".$post->getOwnerID(), "Vote not to banish author", "Vote not to banish author");
      } else {
          show_button("forum_banishment_vote.php?action=start&userid=".$post->getOwnerID(), "Start vote to banish author", "Start vote to banish author");
      }
    }
}

?>
