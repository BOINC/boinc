<?php
// $Id: views_handler_field_markup.inc,v 1.3.4.1 2009/11/18 20:52:23 merlinofchaos Exp $

/**
 * A handler to run a field through check_markup, using a companion
 * format field.
 *
 * - format: (REQUIRED) The field in this table used to control the format
 *           such as the 'format' field in the node, which goes with the
 *           'body' field.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_markup extends views_handler_field {
  /**
   * Constructor; calls to base object constructor.
   */
  function construct() {
    parent::construct();

    $this->format = $this->definition['format'];

    $this->additional_fields = array();
    if (!is_numeric($this->format)) {
      $this->additional_fields['format'] = array('field' => $this->format);
    }
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    $format = is_numeric($this->format) ? $this->format : $values->{$this->aliases['format']};
    if ($value) {
      return check_markup($value, $format, FALSE);
    }
  }

  function element_type() {
    if (isset($this->definition['element type'])) {
      return $this->definition['element type'];
    }

    return 'div';
  }
}
