<?php
/**
 * Allow Group ID(s) as argument
 *
 * @ingroup views_argument_handlers
 */
class og_views_handler_argument_og_uid_nid extends views_handler_argument_numeric {
  // Set context based on first node passed.
  function query() {
    $args = explode(',', $this->argument);
    $node = node_load((int)$args[0]);
    og_set_group_context($node);
    parent::query();
  }
}