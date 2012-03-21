<?php

/**
 * @file flag_friend_handler_argument_numeric.inc
 *
 * Contains the custom argument handler for the flag_friend table.
 */
class flag_friend_handler_argument_numeric extends views_handler_argument_numeric {
  function option_definition() {
    $options = parent::option_definition();
    $options['flag_friend_include_arg_user'] = array(
      'default' => 0,
      'translatable' => FALSE,
    );
    return $options;
  }
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $options = $this->options;
    $form['flag_friend_include_arg_user'] = array(
      '#title' => t('Include argument user'),
      '#description' => t('Include results from the user whose friends will be shown.'),
      '#type' => 'checkbox',
      '#default_value' => $options['flag_friend_include_arg_user'],
    );
  }
  function query() {
    if (!empty($this->options['break_phrase'])) {
      views_break_phrase($this->argument, $this);
    }
    else {
      $this->value = array($this->argument);
    }
    $group = $this->query->set_where_group('OR', 'friends');
    if ($this->options['flag_friend_include_arg_user']) {
      $this->query->add_where($group, "users.uid = %d", $this->argument);
    }
    if (count($this->value) > 1) {
      $operator = empty($this->options['not']) ? 'IN' : 'NOT IN';
      $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));
      $this->query->add_where($group, "users.uid IN (SELECT f.friend_uid FROM {flag_friend} f WHERE f.uid $operator ($placeholders))", $this->value);
      $this->query->add_where($group, "users.uid IN (SELECT f.uid FROM {flag_friend} f WHERE f.friend_uid $operator ($placeholders))", $this->value);
    }
    else {
      $operator = empty($this->options['not']) ? '=' : '!=';
      $this->query->add_where($group, "users.uid IN (SELECT f.friend_uid FROM {flag_friend} f WHERE f.uid $operator %d)", $this->argument);
      $this->query->add_where($group, "users.uid IN (SELECT f.uid FROM {flag_friend} f WHERE f.friend_uid $operator %d)", $this->argument);
    }
  }
}