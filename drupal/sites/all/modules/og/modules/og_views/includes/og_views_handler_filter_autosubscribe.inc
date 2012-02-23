<?php
 /*
 * Field handler to filter by Autosubscribe state
 *
 * @ingroup views_filter_handlers
 */
class og_views_handler_filter_autosubscribe extends views_handler_filter_in_operator {
  function get_value_options() {
    if (isset($this->value_options)) {
      return;
    }

    $this->value_options = array();
    foreach (og_notifications_autosubscribe_map() as $key => $name) {
      $this->value_options[$key] = $name;
    }
  }
}