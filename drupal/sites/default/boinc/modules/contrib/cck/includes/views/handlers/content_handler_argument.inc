<?php
// $Id$

/**
 * @file
 * The subclass simply adds properties,
 * for field-specific subclasses to use if they need to.
 */
class content_handler_argument extends views_handler_argument {
  var $content_field;

  function construct() {
    parent::construct();

    $this->content_field = content_fields($this->definition['content_field_name']);
    $this->additional_fields = $this->definition['additional fields'];
  }
}