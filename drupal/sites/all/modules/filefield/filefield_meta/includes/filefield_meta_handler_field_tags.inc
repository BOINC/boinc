<?php
// $Id: filefield_meta_handler_field_tags.inc,v 1.1 2010/06/16 22:06:44 quicksketch Exp $

/**
 * @file
 * A special handler that renders ID3 tags attached to a file.
 */

/**
 * Render a field as a readable value in hours, minutes, and seconds.
 *
 * @ingroup views_field_handlers
 */
class filefield_meta_handler_field_tags extends views_handler_field {
  function option_definition() {
    $options = parent::option_definition();
    $default = reset(array_keys(filefield_meta_tags()));
    $options['tag'] = array('tag' => $default, 'required' => TRUE, 'translatable' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['relationship']['#weight'] = -2;
    $form['tag'] = array(
      '#type' => 'select',
      '#title' => t('ID3 tag'),
      '#required' => TRUE,
      '#default_value' => $this->options['tag'],
      '#options' => filefield_meta_tags(),
      '#description' => t('Select the tag to be rendered. If needing multiple tags, add another ID3 tags field.'),
      '#weight' => -1,
    );
  }

  function render($values) {
    $value = unserialize($values->{$this->field_alias});
    $tag = $this->options['tag'];
    if (isset($value[$tag])) {
      return check_plain($value[$tag]);
    }
  }
}
