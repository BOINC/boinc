<?php
// TODOL: query() does not work.
class og_views_handler_filter_og_type extends views_handler_filter_node_type {
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Group node type');
      $group_types = og_get_types('group');
      $types = node_get_types();
      foreach ($group_types as $group_type) {
        $options[$group_type] = $types[$group_type]->name;
      }
      $this->value_options = $options;
    }
  }
  
  // This is a copy of views_handler_filter_in_operator::query
  // We force the table to be 'node' instead of 'og'. There might be cleaner ways to do this.
  function query() {
    if (empty($this->value)) {
      return;
    }
    $table = $this->query->ensure_table('node');
    $placeholder = !empty($this->definition['numeric']) ? '%d' : "'%s'";

    $replace = array_fill(0, sizeof($this->value), $placeholder);
    $in = ' (' . implode(", ", $replace) . ')';
    $this->query->add_where($this->options['group'], "$table.$this->real_field " . $this->operator . $in, $this->value);
  } 
}