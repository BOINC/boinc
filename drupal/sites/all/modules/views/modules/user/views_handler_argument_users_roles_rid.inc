<?php
// $Id: views_handler_argument_users_roles_rid.inc,v 1.1 2008/09/03 19:21:30 merlinofchaos Exp $
/**
 * Allow role ID(s) as argument
 */
class views_handler_argument_users_roles_rid extends views_handler_argument_many_to_one {
  function title_query() {
    $titles = array();
    $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));

    $result = db_query("SELECT name FROM {role} WHERE rid IN ($placeholders)", $this->value);
    while ($term = db_fetch_object($result)) {
      $titles[] = check_plain($term->name);
    }
    return $titles;
  }
}

