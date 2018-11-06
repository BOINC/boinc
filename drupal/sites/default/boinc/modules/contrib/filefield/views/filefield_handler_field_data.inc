<?php

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

    $options = array();
    $info = filefield_data_info();
    foreach ($info as $key => $data) {
      $options[$key] = $data['title'] . ' (' . $data['module'] .')';
    }

    $form['data_key'] = array(
      '#title' => t('Data key'),
      '#type' => 'radios',
      '#options' => $options,
      '#required' => TRUE,
      '#default_value' => $this->options['data_key'],
      '#description' => t('The data column may contain only a few or none any of these data options. The name of the module that provides the data is shown in parentheses.'),
      '#weight' => 4,
    );
  }

  function admin_summary() {
    // Display the data to be displayed.
    $info = filefield_data_info();
    return isset($info[$this->options['data_key']]['title']) ? $info[$this->options['data_key']]['title'] : $this->options['data_key'];
  }

  function render($values) {
    $values = drupal_clone($values); // Prevent affecting the original.
    $data = unserialize($values->{$this->field_alias});
    $values->{$this->field_alias} = filefield_data_value($this->options['data_key'], $data[$this->options['data_key']]);

    // Copied from views_handler_field_node(). We just remove the call to
    // sanitize_value() from the original call, becaue our value has already
    // been cleaned by filefield_data_value().
    return $this->render_link($values->{$this->field_alias}, $values);
  }

}
