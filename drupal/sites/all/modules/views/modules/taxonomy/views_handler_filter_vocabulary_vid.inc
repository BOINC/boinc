<?php
// $Id: views_handler_filter_vocabulary_vid.inc,v 1.3 2009/06/10 22:04:58 merlinofchaos Exp $

/**
 * Filter by vocabulary id
 */
class views_handler_filter_vocabulary_vid extends views_handler_filter_in_operator {
  function get_value_options() {
    if (isset($this->value_options)) {
      return;
    }

    $this->value_options = array();
    $vocabularies = taxonomy_get_vocabularies();
    foreach ($vocabularies as $voc) {
      $this->value_options[$voc->vid] = $voc->name;
    }
  }
}
