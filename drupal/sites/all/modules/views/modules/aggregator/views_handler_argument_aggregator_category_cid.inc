<?php
// $Id: views_handler_argument_aggregator_category_cid.inc,v 1.1 2009/06/02 19:46:06 merlinofchaos Exp $

/**
 * Argument handler to accept an aggregator category id.
 */
class views_handler_argument_aggregator_category_cid extends views_handler_argument_numeric {
  /**
   * Override the behavior of title(). Get the title of the category.
   */
  function title_query() {
    $titles = array();
    $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));

    $result = db_query("SELECT c.title FROM {aggregator_category} c WHERE c.cid IN ($placeholders)", $this->value);
    while ($term = db_fetch_object($result)) {
      $titles[] = check_plain($term->title);
    }
    return $titles;
  }
}
