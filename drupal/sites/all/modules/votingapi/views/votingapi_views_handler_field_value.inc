<?php
// $Id: votingapi_views_handler_field_value.inc,v 1.1.2.3 2009/06/21 01:10:39 eaton Exp $

/**
 * @file
 * Provide a views handlers for votingapi data fields.
 */

class votingapi_views_handler_field_value extends views_handler_field_numeric {
  function construct() {
    parent::construct();
    $this->definition['float'] = TRUE;
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['appearance'] = '';
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $appearances = module_invoke_all('votingapi_views_formatters', $this);
    $options = array('' => t('Default appearance'));
    $options += $appearances;

    if (count($options) > 1) {
      $form['appearance'] = array(
        '#type' => 'select',
        '#title' => t('Appearance'),
        '#options' => $options,
        '#default_value' => $this->options['appearance'],
        '#weight' => -5,
      );
    }
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    $function = $this->options['appearance'];
    if (!empty($function) && function_exists($function)) {
      return $function($value, $this, $values);
    }
    else {
      return parent::render($values);
    }
  }
}
