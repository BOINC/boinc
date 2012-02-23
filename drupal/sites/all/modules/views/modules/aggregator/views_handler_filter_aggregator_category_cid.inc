<?php
// $Id: views_handler_filter_aggregator_category_cid.inc,v 1.2 2009/06/10 22:04:57 merlinofchaos Exp $

/**
 * Filter by aggregator category cid
 */
class views_handler_filter_aggregator_category_cid extends views_handler_filter_in_operator {
  function get_value_options() {
    if (isset($this->value_options)) {
      return;
    }

    $this->value_options = array();

    $result = db_query('SELECT * FROM {aggregator_category} ORDER BY title');
    while ($category = db_fetch_object($result)) {
      $this->value_options[$category->cid] = $category->title;
    }
  }
}
