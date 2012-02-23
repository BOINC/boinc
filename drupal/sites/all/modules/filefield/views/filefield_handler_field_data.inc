<?php
// $Id: filefield_handler_field_data.inc,v 1.2 2010/01/08 22:38:40 quicksketch Exp $

/**
 * @file
 * filefield_handler_field_data.inc
 *
 * Provides a handler for displaying values within the serialized data column.
 */
class filefield_handler_field_data extends views_handler_field_node {

  function option_definition() {
    $options = parent::option_definition();
    $options['data_key'] = array('default' => 'description');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['data_key'] = array(
      '#title' => t('Data key'),
      '#type' => 'radios',
      // TODO: Pull these values from the modules that extend FileField.
      '#options' => drupal_map_assoc(array('description', 'title', 'alt')),
      '#required' => TRUE,
      '#default_value' => $this->options['data_key'],
      '#description' => t('The data column may (or may not) contain any of the following data. Select the data that should be output for this field.'),
      '#weight' => 4,
    );
  }

  function admin_summary() {
    // Display the data to be displayed
    return $this->options['data_key'];
  }

  function render($values) {
    $values = drupal_clone($values); // Prevent affecting the original.
    $data = unserialize($values->{$this->field_alias});
    $values->{$this->field_alias} = $data[$this->options['data_key']];
    return parent::render($values);
  }

}
