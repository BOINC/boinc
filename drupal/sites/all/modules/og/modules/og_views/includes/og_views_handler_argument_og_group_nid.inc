<?php
class og_views_handler_argument_og_group_nid extends views_handler_argument_numeric {
  // Set context based on first node passed.
  function query() {
    $args = explode(',', $this->argument);
    $node = node_load((int)$args[0]);
    og_set_group_context($node);
    parent::query();
  }
  
  /**
   * Override the behavior of title(). Get the title of the node.
   */
  function title_query() {
    $titles = array();
    $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));

    $result = db_query("SELECT n.title FROM {node} n WHERE n.nid IN ($placeholders)", $this->value);
    while ($term = db_fetch_object($result)) {
      $titles[] = check_plain($term->title);
    }
    return $titles;
  }
}