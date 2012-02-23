<?php
// $Id: content_handler_filter_many_to_one.inc,v 1.1.2.5 2009/08/14 19:15:10 markuspetrux Exp $

/**
 * @file
 * The subclass simply adds properties,
 * for field-specific subclasses to use if they need to.
 */
class content_handler_filter_many_to_one extends views_handler_filter_many_to_one {
  var $content_field;

  function construct() {
    parent::construct();
    $this->content_field = content_fields($this->definition['content_field_name']);
    $this->additional_fields = $this->definition['additional fields'];
    $field = $this->content_field;
    $this->value_title = $field['widget']['label'];
  }

  function get_value_options() {
    $this->value_options = $this->allowed_values();
  }

  // Get allowed values from hook_allowed_values(), if any,
  // or from content_allowed_values();
  function allowed_values() {
    $field = $this->content_field;
    $function = $field['module'] .'_allowed_values';
    if ($this->value_form_type == 'select') {
      // Select elements accept multidimensional arrays to support optgroups.
      $options = function_exists($function) ? $function($field) : content_allowed_values($field, FALSE);
      // For selects, HTML should be filtered out and entities left unencoded.
      // See content_allowed_values / content_filter_xss / filter_xss.
      content_allowed_values_filter_html($options);
    }
    else {
      $options = function_exists($function) ? $function($field) : content_allowed_values($field);
    }
    return (array) $options;
  }

}