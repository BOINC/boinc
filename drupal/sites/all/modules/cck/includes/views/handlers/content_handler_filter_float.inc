<?php
// $Id: content_handler_filter_float.inc,v 1.1.2.3 2009/04/09 00:04:23 yched Exp $

/**
 * @file
 * The subclass simply adds properties,
 * for field-specific subclasses to use if they need to.
 */

// Ensure compatibility with Views pre 2.4, where the
// views_handler_filter_float class lived in views_handler_filter_numeric.inc.
if (!class_exists('views_handler_filter_float')) {
  // Manually include the parent class.
  $definition = views_fetch_handler_data('views_handler_filter_numeric');
  views_include_handler($definition, 'handler');
}

class content_handler_filter_float extends views_handler_filter_float {
  var $content_field;

  function construct() {
    parent::construct();
    $this->content_field = content_fields($this->definition['content_field_name']);
    $this->additional_fields = $this->definition['additional fields'];
  }
}