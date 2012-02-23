<?php
/**
 * Field handler to show Autosubscribe state.
 *
 * @ingroup views_field_handlers
 */
class og_views_handler_field_autosubscribe extends views_handler_field {
  function render($values) {
    $map = og_notifications_autosubscribe_map();
    return $map[$values->{$this->field_alias}];
  }
}