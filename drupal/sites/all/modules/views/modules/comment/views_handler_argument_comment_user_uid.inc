<?php
// $Id: views_handler_argument_comment_user_uid.inc,v 1.2 2009/06/02 18:50:56 merlinofchaos Exp $

/**
 * Argument handler to accept a user id to check for nodes that
 * user posted or commented on.
 */
class views_handler_argument_comment_user_uid extends views_handler_argument {
  function title() {
    if (!$this->argument) {
      $title = variable_get('anonymous', t('Anonymous'));
    }
    else {
      $title = db_result(db_query("SELECT u.name FROM {users} u WHERE u.uid = %d", $this->argument));
    }
    if (empty($title)) {
      return t('No user');
    }

    return check_plain($title);
  }

  function default_actions($which = NULL) {
    // Disallow summary views on this argument.
    if (!$which) {
      $actions = parent::default_actions();
      unset($actions['summary asc']);
      unset($actions['summary desc']);
      return $actions;
    }

    if ($which != 'summary asc' && $which != 'summary desc') {
      return parent::default_actions($which);
    }
  }

  function query() {
    $this->ensure_my_table();
    $this->query->add_where(0, "$this->table_alias.uid = %d OR ((SELECT COUNT(*) FROM {comments} c WHERE c.uid = %d AND c.nid = $this->table_alias.nid) > 0)", $this->argument, $this->argument);
  }
}
