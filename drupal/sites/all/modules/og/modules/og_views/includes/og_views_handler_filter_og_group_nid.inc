<?php
class og_views_handler_filter_og_group_nid extends views_handler_filter_many_to_one {
  function get_value_options() {
    $this->value_options = $this->allowed_values();
  }

  // Get allowed values from hook_allowed_values(), if any,
  // or from content_allowed_values();
  function allowed_values() {
    return og_all_groups_options();
  }
  
  function query() {
    if (isset($this->value['value'])) {
      $node = node_load((int)$this->value[0]);
      og_set_group_context($node);
    }
    parent::query();
  }
}