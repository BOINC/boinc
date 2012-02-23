<?php
// $Id: views_handler_argument_aggregator_fid.inc,v 1.1 2009/01/07 19:38:08 merlinofchaos Exp $

/**
 * Argument handler to accept an aggregator feed id.
 */
class views_handler_argument_aggregator_fid extends views_handler_argument_numeric {
  /**
   * Override the behavior of title(). Get the title of the feed.
   */
  function title_query() {
    $titles = array();
    $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));

    $result = db_query("SELECT f.title FROM {aggregator_feed} f WHERE f.fid IN ($placeholders)", $this->value);
    while ($term = db_fetch_object($result)) {
      $titles[] = check_plain($term->title);
    }
    return $titles;
  }
}

