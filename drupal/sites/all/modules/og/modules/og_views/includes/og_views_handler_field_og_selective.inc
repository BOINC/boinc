<?php
/**
 * Field handler to show Selective state.
 *
 * @ingroup views_field_handlers
 */
class og_views_handler_field_og_selective extends views_handler_field {
  function render($values) {
    $map = og_selective_map();
    return $map[$values->{$this->field_alias}];
  }
}