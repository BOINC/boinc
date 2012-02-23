<?php
// $Id: views_customfield_handler_field_markup.inc,v 1.1.2.5 2008/11/19 13:53:47 casey Exp $

/**
 * @file
 * Contains the 'customfield' markup field handler.
 */

/**
 * Field handler to display custom markup text.
 */
class views_customfield_handler_field_markup extends views_handler_field {

  function query() {
    $this->field_alias = 'customfield_markup_'. $this->position;
    
    $value = db_escape_string($this->options['value']);
    $this->query->add_field('', "'$value'", $this->field_alias);
  }
  
  function option_definition() {
    $options = parent::option_definition();
    $options['value'] = array('default' => '');
    $options['format'] = array('default' => FILTER_FORMAT_DEFAULT);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['value'] = array(
      '#type' => 'textarea',
      '#title' => t('Value'),
      '#default_value' => $this->options['value'],
      '#rows' => 5,
    );
    $form['format'] = filter_form($this->options['format'], NULL, array('options', 'format'));
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    return check_markup($value, $this->options['format'], FALSE);
  }
}
