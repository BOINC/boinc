<?php
/**
 * @file
 * Contains the php code argument default plugin.
 */

/**
 * Default argument plugin to provide a PHP code block.
 */
class views_plugin_argument_default_php extends views_plugin_argument_default {
  var $option_name = 'default_argument_php';

  function argument_form(&$form, &$form_state) {
    $form[$this->option_name] = array(
      '#type' => 'textarea',
      '#title' => t('PHP argument code'),
      '#default_value' => $this->get_argument(TRUE), // the true forces it raw.
      '#process' => array('views_process_dependency'),
      '#description' => t('Enter PHP code that returns a value to use for this argument. Do not use &lt;?php ?&gt;. You must return only a single value for just this argument.'),
      '#dependency' => array(
        'radio:options[default_action]' => array('default'),
        'radio:options[default_argument_type]' => array($this->id)
      ),
      '#dependency_count' => 2,
    );

    $this->check_access($form);
  }

  /**
   * Only let users with PHP block visibility permissions set/modify this
   * default plugin.
   */
  function access() {
    return user_access('use PHP for block visibility');
  }

  function get_argument($raw = FALSE) {
    if ($raw) {
      return parent::get_argument();
    }

    // set up variables to make it easier to reference during the argument.
    $view = &$this->view;
    $argument = &$this->argument;
    ob_start();
    $result = eval($this->argument->options[$this->option_name]);
    ob_end_clean();
    return $result;
  }
}
