<?php
// $Id: views_handler_filter_user_current.inc,v 1.2 2009/09/15 16:18:38 merlinofchaos Exp $

/**
 * Filter handler for the current user
 */
class views_handler_filter_user_current extends views_handler_filter_boolean_operator {
  function construct() {
    parent::construct();
    $this->value_value = t('Is the logged in user');
  }

  function query() {
    $this->ensure_my_table();
    $where = "$this->table_alias.$this->real_field ";

    if (empty($this->value)) {
      $where .= '<> ***CURRENT_USER***';
      if ($this->accept_null) {
        $where = '(' . $where . " OR $this->table_alias.$this->real_field IS NULL)";
      }
    }
    else {
      $where .= '= ***CURRENT_USER***';
    }
    $this->query->add_where($this->options['group'], $where);    
  }
}
