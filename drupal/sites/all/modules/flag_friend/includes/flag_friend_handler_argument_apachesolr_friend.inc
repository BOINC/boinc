<?php
//$ Id: $
/**
 * @file
 * provides Views implementation for the apachesolr Views project
 * filters the results by content authored by the users friends
 */
class flag_friend_handler_argument_apachesolr_friend extends apachesolr_views_handler_argument {
  function query() {
     if (!empty($this->options['break_phrase'])) {
      $this->value = explode(',', $this->argument);
    }
    else {
      $this->value = array($this->argument);
    }
    $filter_string = '';
    foreach ($this->value as $uid) {
      foreach (flag_friend_get_friends($uid) as $friend_id => $friend) {
        $filter_string .= " OR $friend_id";
      }
    }
    $filter_string = substr($filter_string, 4);
    $this->query->add_filter($this->real_field, "($filter_string)");
  }
}
?>