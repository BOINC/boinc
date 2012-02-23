<?php
// $Id: flag_friend_handler_argument_numeric.inc,v 1.1.2.3 2009/04/30 23:25:56 sirkitree Exp $

/**
 * @file flag_friend_handler_argument_numeric.inc
 *
 * Contains the custom argument handler for the flag_friend table.
 */
class flag_friend_handler_argument_numeric extends views_handler_argument_numeric {
  function query() {
    $this->ensure_my_table();

    if (!empty($this->options['break_phrase'])) {
      views_break_phrase($this->argument, $this);
    }
    else {
      $this->value = array($this->argument);
    }
    $group = $this->query->set_where_group('OR', 'friends');
    if (count($this->value) > 1) {
      $operator = empty($this->options['not']) ? 'IN' : 'NOT IN';
      $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));
      $this->query->add_where($group, "users.uid IN (SELECT f.friend_uid FROM {flag_friend} f WHERE f.uid $operator ($placeholders))", $this->value);
      $this->query->add_where($group, "users.uid IN (SELECT f.uid FROM {flag_friend} f WHERE f.friend_uid $operator ($placeholders))", $this->value);
    }
    else {
      $operator = empty($this->options['not']) ? '=' : '!=';
      $this->query->add_where($group, "users.uid IN (SELECT f.friend_uid FROM {flag_friend} f WHERE f.uid $operator %d)", $this->argument);
      $this->query->add_where($group, "users.uid IN (SELECT f.uid FROM {flag_friend} f WHERE f.friend_uid $operator %d)", $this->argument);
    }
  }
}