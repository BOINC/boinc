<?php
// $Id: views_handler_field_comment_link.inc,v 1.2 2008/09/10 00:16:14 merlinofchaos Exp $
/**
 * Base field handler to present a link.
 */
class views_handler_field_comment_link extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['cid'] = 'cid';
    $this->additional_fields['nid'] = 'nid';
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
    $text = !empty($this->options['text']) ? $this->options['text'] : t('view');
    return l($text, "node/" . $values->{$this->aliases['nid']}, array('html' => TRUE, 'fragment' => "comment-" . $values->{$this->aliases['cid']}));
  }
}

