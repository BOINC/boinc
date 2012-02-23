<?php
// $Id: views_handler_field_locale_link_edit.inc,v 1.1 2009/02/20 23:02:09 merlinofchaos Exp $

/**
 * Field handler to present a link to edit a translation.
 */
class views_handler_field_locale_link_edit extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['lid'] = 'lid';
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['text'] = array('default' => '', 'translatable' => TRUE);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
    );
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    // Ensure user has access to edit translations.
    if (!user_access('translate interface')) {
      return;
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('edit');
    return l($text, 'admin/build/translate/edit/' . $values->{$this->aliases['lid']}, array('query' => drupal_get_destination()));
  }
}

