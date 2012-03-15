<?php
// $Id: context_condition_book.inc,v 1.1.2.4 2010/08/03 03:25:23 yhahn Exp $

/**
 * Expose book properties as a context condition.
 */
class context_condition_book extends context_condition {
  function condition_values() {
    $values = array();
    foreach (book_get_books() as $book) {
      $values[$book['menu_name']] = check_plain($book['title']);
    }
    return $values;
  }

  function execute($node, $op) {
    if (isset($node->book, $node->book['menu_name'])) {
      foreach ($this->get_contexts($node->book['menu_name']) as $context) {
        $this->condition_met($context, $node->book['menu_name']);
      }
    }
  }
}
