<?php
// $Id: views_plugin_argument_default_user.inc,v 1.1 2008/09/03 19:21:30 merlinofchaos Exp $
/**
 * @file
 * Contains the user from URL argument default plugin.
 */

/**
 * Default argument plugin to extract a user via menu_get_object
 */
class views_plugin_argument_default_user extends views_plugin_argument_default {
  var $option_name = 'default_argument_user';

  function argument_form(&$form, &$form_state) {
    $form[$this->option_name] = array(
      '#type' => 'checkbox',
      '#title' => t('Also look for a node and use the node author'),
      '#default_value' => !empty($this->argument->options[$this->option_name]),
      '#process' => array('views_process_dependency'),
      '#dependency' => array(
        'radio:options[default_action]' => array('default'),
        'radio:options[default_argument_type]' => array($this->id)
      ),
      '#dependency_count' => 2,
    );
  }

  function get_argument() {
    foreach (range(1, 3) as $i) {
      $user = menu_get_object('user', $i);
      if (!empty($user)) {
        return $user->uid;
      }
    }

    foreach (range(1, 3) as $i) {
      $user = menu_get_object('user_uid_optional', $i);
      if (!empty($user)) {
        return $user->uid;
      }
    }

    if (!empty($this->argument->options[$this->option_name])) {
      foreach (range(1, 3) as $i) {
        $node = menu_get_object('node', $i);
        if (!empty($node)) {
          return $node->uid;
        }
      }
    }

    if (arg(0) == 'user' && is_numeric(arg(1))) {
      return arg(1);
    }

    if (arg(0) == 'node' && is_numeric(arg(1))) {
      $node = node_load(arg(1));
      if ($node) {
        return $node->uid;
      }
    }
  }
}

