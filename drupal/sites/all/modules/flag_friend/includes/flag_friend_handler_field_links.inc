<?php

/**
 * @file
 * Contains the flag friend Ops field handler.
 */

/**
 * Views field handler for the Flag friend operations links (remove/pending/approve/deny).
 *
 * @ingroup views
 */
class flag_friend_handler_field_links extends views_handler_field {
  function render($values) {
    global $user;
    $flag = flag_get_flag('friend');
    $content_id = $values->uid;
    // what's the status?
    $status = flag_friend_determine_friend_status($flag, $user->uid, $content_id);
    $link_type = ($status == FLAG_FRIEND_APPROVAL) ? 'unflag' : 'unfriend';
    if ($this->view->args[0] == $user->uid) {
      return flag_friend_create_link($link_type, $content_id);
    }
  }
}
