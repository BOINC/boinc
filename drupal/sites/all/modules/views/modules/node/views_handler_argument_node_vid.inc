<?php
// $Id: views_handler_argument_node_vid.inc,v 1.1 2008/09/03 19:21:29 merlinofchaos Exp $
/**
 * @file
 * Provide node vid argument handler.
 */

/**
 * Argument handler to accept a node revision id.
 */
class views_handler_argument_node_vid extends views_handler_argument_numeric {
  // No constructor is necessary.

  /**
   * Override the behavior of title(). Get the title of the revision.
   */
  function title_query() {
    $titles = array();
    $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));

    $result = db_query("SELECT n.title FROM {node_revisions} n WHERE n.nid IN ($placeholders)", $this->value);
    while ($term = db_fetch_object($result)) {
      $titles[] = check_plain($term->title);
    }
    return $titles;
  }
}

