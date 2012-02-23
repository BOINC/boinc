<?php
// $Id: views_handler_field_user_link.inc,v 1.1 2008/09/03 19:21:30 merlinofchaos Exp $
/**
 * Field handler to present a link to the user.
 */
class views_handler_field_user_link extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = 'uid';
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

  // An example of field level access control.
  function access() {
    return user_access('access user profiles');
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $text = !empty($this->options['text']) ? $this->options['text'] : t('view');
    $uid = $values->{$this->aliases['uid']};
    return l($text, "user/$uid");
  }
}

