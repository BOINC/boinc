<?php
// $Id: views_handler_argument_node_nid.inc,v 1.1 2008/09/03 19:21:29 merlinofchaos Exp $
/**
 * @file
 * Provide node nid argument handler.
 */

/**
 * Argument handler to accept a node id.
 */
class views_handler_argument_node_nid extends views_handler_argument_numeric {
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

