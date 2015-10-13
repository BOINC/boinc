<?php

class content_negotiation_filter_handler extends views_handler_filter {
  function query() {
    $this->ensure_my_table();
    $where = i18n_db_rewrite_where($this->table_alias, 'node');
    if (!empty($where)) {
      $this->query->add_where($this->options['group'], $where);
    }
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['operator']['default'] = '';
    $options['value']['default'] = '';
    return $options;
  }
}
